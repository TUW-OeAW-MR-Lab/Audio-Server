/*
    MatrixController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <util/ListenerInterface.h>
#include <nlohmann/json.hpp>

namespace mrlab::controller
{
class OscController;
class MatrixOscAgent;

//==============================================================================
/** Remote controller for the DirectOut Prodigy.MP audio matrix.

    This attempts to implement a remote control for the prodigy audio
    matrix controller using the JSON network protocol as documented in
    DirectOut's "PRODIGY.MC - JSON Protocol Specifications v1.0".

    This class implements connection handling and the TCP-based
    low-level communication with the Prodigy matrix while the actual
    interface between OSC and the Prodigy's JSON protocol is done in
    the MatrixOscAgent class.

    Prospectively, this shall also implement the meter data querying network
    protocol as documented in "PRODIGY Levelmeter Stream".

    @see MatrixOscAgent
 */
class MatrixController : private juce::Timer
{
public:
    //==============================================================================
    /** State flags. */
    enum class State
    {
        init         = 0x0,     ///< Initial state, no connection attempt made yet.
        disconnected,           ///< Disconnected (after connection was closed or failed).
        waitingToReconnect,     ///< Waiting for reconnection attempt.
        disconnecting,          ///< Disconnection in progress.
        connecting,             ///< Attempting to connect.
        connected,              ///< Currently connected.
    };


    struct StateDescription
    {
        static std::string_view get (State code)
        {
            return description.at (code);
        }

        /** State values to display strings. */
        inline static const auto description = std::map<State, std::string_view> (
            { { State::init, "" },
              { State::disconnected, "disconnected" },
              { State::waitingToReconnect, "waiting to reconnect..." },
              { State::disconnecting, "disconnecting..." },
              { State::connecting, "connecting..." },
              { State::connected, "connected" } }
        );
    };

    //==============================================================================
    /** Listener interface. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Indicates that the connection state changed.

            @param controller Controller whose state changed.
            @param newState New state of the controller.
         */
        virtual void stateChanged (MatrixController& controller, State newState) = 0;
    };

    //==============================================================================
    MatrixController (OscController& oscControllerIn);
    ~MatrixController() override;

    /** Get the current connection state of this controller.

        @returns The current connection state.
     */
    State getState() const { return state; }

    /** Attempt to connect to the Prodigy matrix.

        @return true if matrix communication thread could be launched, false otherwise.

        @note This is an asynchronous operation, thus the return value
              does not indicate a successful connection. Updates are
              notified via the state listener interface.
     */
    bool connect();

    /** Disconnect from the Prodigy matrix.

        @param reconnectAfterTimeout If true, attempt to reconnect after the
                                     timeout interval determined in Globals.

        @return true on success, false if communication thread could not be stopped.
     */
    bool disconnect (bool reconnectAfterTimeout = false);

    /** Send a Prodigy JSON control message to the matrix.

        @param msg JSON value representing the message-
        @returns true if message could be enqueued for sending, false if not connected.
     */
    template <std::common_reference_with<nlohmann::json> JsonType>
    bool sendMatrixMessage (JsonType&& msg)
    {
        if (state != State::connected)
            return false;

        matrixThread.enqueue (std::forward<JsonType> (msg));

        return true;
    }

private:
    //==============================================================================
    static constexpr auto socketTimeoutMs = 100;
    static constexpr auto threadTimeoutMs = 300;
    static constexpr auto bufferSizeBytes = 65536;

    //==============================================================================
    /** Dispatch a received matrix message to OscAgent. */
    void received (nlohmann::json&& msg);

    /** Checks for worker thread status and updates controller state. */
    void timerCallback() override;

    /** Sets the controller state and notifies listeners. */
    void setStateAndNotify (State newState);

    //==============================================================================
    /** Worker thread for matrix network communication. */
    class MatrixThread : public juce::Thread
    {
    public:
        //==============================================================================
        MatrixThread (juce::StreamingSocket& socketIn, std::function<void (nlohmann::json&&)>&& receivedIn);

        /** Thread function. */
        void run() override;

        /** Enqueue a JSON message for sending. */
        template <std::common_reference_with<nlohmann::json> JsonType>
        void enqueue (JsonType&& msg)
        {
            std::scoped_lock lock (sendQueueAccess);

            sendQueue.push (std::forward<JsonType> (msg));
            sendRequest.store (true);
        }

    private:
        //==============================================================================
        bool send();    ///< Low-level sending implementation.
        bool receive(); ///< Low-level receiving implementation.

        //==============================================================================
        juce::StreamingSocket& socket;                   ///< Reference to the socket used.
        std::function<void (nlohmann::json&&)> received; ///< Callback with received message.
        std::array<char, bufferSizeBytes> buffer;        ///< Receive buffer for reading chunks.
        std::string recvMsg;                             ///< JSON string of received message.
        std::string sendMsg;                             ///< JSON string of message to be sent.
        std::queue<nlohmann::json> sendQueue;            ///< Queue for sending.
        std::mutex sendQueueAccess;                      ///< Lock for sending queue access.
        std::atomic<bool> sendRequest { false };         ///< Flag to indicate a send request.
    };

    //==============================================================================
    OscController& oscController; ///< OscController reference.
    State state;                  ///< Current connection state.
    juce::StreamingSocket socket; ///< Socket used for TCP communication.
    MatrixThread matrixThread;    ///< Thread used for communicating with the matrix.
    std::unique_ptr<MatrixOscAgent> oscAgent; ///< OSC agent for this.

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatrixController)
};

} // namespace mrlab::controller
