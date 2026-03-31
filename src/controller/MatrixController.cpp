/*
    MatrixController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "MatrixController.h"
#include "MatrixControllerOscAgent.h"
#include "MatrixOscAgent.h"
#include <Globals.h>
#include <util/Logger.h>

namespace mrlab::controller
{
MatrixController::MatrixController (OscController& oscControllerIn)
    : oscController (oscControllerIn),
      matrixThread (socket, [this] (nlohmann::json&& msg) {
          juce::MessageManager::callAsync ([this, m = std::move (msg)] () mutable {
              received (std::move (m));
          });
      })
{}

MatrixController::~MatrixController()
{
    disconnect();
}

void MatrixController::initOscAgent()
{
    controllerOscAgent = std::make_unique<MatrixControllerOscAgent> (oscController, *this);
}

bool MatrixController::connect()
{
    if (state == State::connected || state == State::connecting)
        return true;

    stopTimer(); // In case of pending auto-reconnect.

    if (matrixThread.isThreadRunning())
    {
        Logger::logError ("MatrixController::connect(): State is not reported as connected but matrix worker thread is running.");
        return false;
    }

    // Connecting is handled by the worker thread.
    const auto success = matrixThread.startThread();

    if (success)
    {
        matrixOscAgent = std::make_unique<MatrixOscAgent> (oscController, *this);
        startTimer (threadTimeoutMs);
    }

    setStateAndNotify (success ? State::connecting : State::disconnected);

    return success;
}

bool MatrixController::disconnect (bool reconnectAfterTimeout)
{
    if (state != State::connected && state != State::connecting && state != State::waitingToReconnect)
        return true;

    stopTimer();
    matrixOscAgent.reset();

    auto result = matrixThread.stopThread (threadTimeoutMs);

    if (! result)
        Logger::logError ("MatrixController::disconnect(): Stopping worker thread timed out.");

    jassert (! socket.isConnected());
    socket.close(); // Does not harm in case.

    if (reconnectAfterTimeout)
        startTimer (Globals::getMatrixAutoReconnectIntervalMs());

    setStateAndNotify (reconnectAfterTimeout ? State::waitingToReconnect : State::disconnected);

    return result;
}

void MatrixController::received (nlohmann::json&& msg)
{
    if (matrixOscAgent)
        matrixOscAgent->handleMatrixMessage (std::move (msg));
}

void MatrixController::timerCallback()
{
    // Disconnect if anything is wrong with socket state or the worker threads.
    if (state == State::connected && ! (socket.isConnected() && matrixThread.isThreadRunning()))
    {
        disconnect (true);
        return;
    }

    // If waiting for reconnect...
    if (state == State::waitingToReconnect)
    {
        connect();
        return;
    }

    // If waiting for connection...
    if (state == State::connecting)
    {
        // ... check whether we are ready.
        if (socket.isConnected())
        {
            setStateAndNotify (State::connected);
            return;
        }

        // If worker thread did not make it: connection failed.
        if (! matrixThread.isThreadRunning())
        {
            Logger::logWarn ("MatrixController: Connecting to Prodigy audio matrix failed.");
            disconnect (true);
        }
    }
}

void MatrixController::setStateAndNotify (State newState)
{
    if (state == newState)
        return;

    state = newState;
    listeners.call (&Listener::stateChanged, *this, state);

    Logger::logInfo (juce::String ("MatrixController: State change to ") +
                     juce::String (int32_t (state)) +
                     " (" + StateDescription::get (state).data() + ").");
}

//==============================================================================
// MatrixThread implementation from here
//==============================================================================
MatrixController::MatrixThread::MatrixThread (juce::StreamingSocket& socketIn,
                                              std::function<void (nlohmann::json&&)>&& receivedIn)
    : juce::Thread ("Prodigy matrix worker thread"),
      socket (socketIn),
      received (std::move (receivedIn))
{}

void MatrixController::MatrixThread::run()
{
    jassert (! socket.isConnected());

    // Connect first.
    if (! socket.connect (Globals::getMatrixHost(), Globals::getMatrixControlPort()))
        return;

    // Send/receive loop.
    while (true)
    {
        if (! send())
            break; // error/connection closed

        if (threadShouldExit())
            break;

        if (! receive())
            break; // error/connection closed

        if (threadShouldExit())
            break;
    }

    socket.close();
}

bool MatrixController::MatrixThread::send()
{
    // Check whether there is something to send.
    while (sendRequest.load())
    {
        const auto ready = socket.waitUntilReady (false, socketTimeoutMs);

        if (ready == 0)
            return true; // timeout, try later
        if (ready == -1)
            return false; // error

        nlohmann::json msg;

        {
            std::scoped_lock lock (sendQueueAccess);

            msg = std::move (sendQueue.front());
            sendQueue.pop();

            if (sendQueue.empty())
                sendRequest.store (false);
        }

        // Serialize JSON message.
        try
        {
            sendMsg = msg.dump();
        }
        catch (const std::runtime_error& e)
        {
            Logger::logError (juce::String ("MatrixController::send(): Ignoring JSON message (") + e.what() + ").");
            continue;
        }

        sendMsg.append ("\n"); // Newline separator according to Prodigy documentation.

        auto bytesToWrite = int (sendMsg.size());
        auto* data = sendMsg.data();

        while (bytesToWrite > 0)
        {
            auto bytesWritten = socket.write (data, bytesToWrite);

            if (bytesWritten == -1)
                return false; // error

            data += bytesWritten;
            bytesToWrite -= bytesWritten;
        }
    }

    return true;
}

bool MatrixController::MatrixThread::receive()
{
    while (true)
    {
        // Check whether there is something to read.
        const auto isReady = socket.waitUntilReady (true, socketTimeoutMs);

        if (isReady == 0)
            return true; // timeout, try again later
        if (isReady == -1)
            return false; // error

        const auto bytesRead = socket.read (buffer.data(), bufferSizeBytes, false);

        /* Reading 0 bytes usually happens after the socket has been
           closed by the remote peer.
        */
        if (bytesRead == 0)
            return false; // closed by peer
        if (bytesRead == -1)
            return false; // error

        /* According to Prodigy documentation, messages shall be
           separated by newlines (\n). Note that TCP is stream-based,
           so network transport may happen in any chunks, across
           message bounds. The implementation below aims to cover all
           cases (multiple messages in one receive buffer, one message
           spanning across several buffers ec.). In the (probably)
           common case of one message per receive buffer, this
           universal solution will employ a no-op std::find
           (recvData.end(), recvData.end()) plus a no-op
           message.append (recvData.end(), recvData.end()), but
           maybe we can live with that.
        */
        auto recvData = std::span (buffer.begin(), size_t (bytesRead));
        auto start = recvData.begin();
        auto nl = start;

        // Look for JSON message boundaries (newline delimiter).
        while ((nl = std::find (start, recvData.end(), '\n')) != recvData.end())
        {
            recvMsg.append (start, ++nl); // Copy message including newline.

            // Parse and dispatch message.
            try
            {
                auto msg = nlohmann::json::parse (recvMsg);
                received (std::move (msg));
            }
            catch (const std::runtime_error& e)
            {
                Logger::logError (juce::String ("MatrixController::receive(): Ignoring JSON message (") + e.what() + ").");
            }

            // Start over with new message.
            recvMsg.clear();
            start = nl;
        }

        // Copy the rest of the received data.
        jassert (nl == recvData.end());
        recvMsg.append (start, nl);
    }
}

} // namespace mrlab::controller
