/*
    AppHandle.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppHandle.h"

namespace mrlab::controller
{

AppHandle::AppHandle (AppConfig newConfig)
    : config (std::move (newConfig))
{}

AppHandle::AppState AppHandle::start()
{
    if (isRunning() || process.isRunning())
        return state;

    int streamFlags = (config.captureStdOut ? juce::ChildProcess::wantStdOut : 0) |
                      (config.captureStdErr ? juce::ChildProcess::wantStdErr : 0);

    // Set app working directory (save and restore current one).
    auto currentWorkingDir = juce::File::getCurrentWorkingDirectory();
    // TODO: Check for success (IMRV-53).
    config.workingDir.setAsCurrentWorkingDirectory();

    setStateAndNotify (process.start (config.startCommand, streamFlags) ? AppState::alive : AppState::startFailed);

    currentWorkingDir.setAsCurrentWorkingDirectory();

    startTimerHz (stateUpdateHz);

    // Start reading the output asynchronously as it is blocking.
    if (streamFlags)
    {
        auto readOutput = [&] (std::stop_token stopToken) {

            std::array<std::byte, 512> buffer;

            while (auto numRead = process.readProcessOutput (buffer.data(), int (buffer.size())))
            {
                if (stopToken.stop_requested())
                    return;

                juce::ScopedLock sl (outputLock);
                output.write (buffer.data(), size_t (numRead));
            }
        };

        jassert (captureThread == nullptr); // There seems to be a spurious capture thread!
        captureThread = std::make_unique<std::jthread> (std::move (readOutput));
    }

    // TODO: implement waiting for app to become ready (IMRV-33)
    // for now, we just become ready 3 seconds after launching.
    juce::Timer::callAfterDelay (3000, [&] {
        if (state != AppState::alive)
            return;

        setStateAndNotify (AppState::ready);
    });

    return state;
}

AppHandle::AppState AppHandle::stop()
{
    if (! isRunning())
        return state;

    if (config.stopCommand.isEmpty())
        kill();

    // TODO: implement app stop request (IMRV-32)

    setStateAndNotify (AppState::stopRequested);

    return state;
}


AppHandle::AppState AppHandle::kill()
{
    if (process.isRunning())
        setStateAndNotify (process.kill() ? AppState::killRequested : AppState::killFailed);

    return state;
}

uint32_t AppHandle::getExitCode() const
{
    jassert (isFinished());

    return process.getExitCode();
}

void AppHandle::timerCallback()
{
    updateState();

    if (! isRunning())
        stopTimer();

    updateOutput();
}

void AppHandle::setStateAndNotify (AppState newState)
{
    if (state == newState)
        return;

    state = newState;
    listeners.call (&Listener::appStateChanged, *this, state);
}

void AppHandle::updateState()
{
    const auto processRunning = process.isRunning();

    if (processRunning)
    {
        // note: process.isRunning() != this.isRunning()!
        jassert (isRunning()); // App state indicates a non-running state although process is still alive!
        return;
    }

    // Process stopped: check previous state and determine what to do.
    if (state == AppState::killRequested)
        setStateAndNotify (AppState::killed);
    else if (isRunning())
        setStateAndNotify (process.getExitCode() == 0 ? AppState::stoppedSuccess : AppState::stoppedError);

    captureThread.reset();
    // TODO: Implement crash detection of app (IMRV-35).
}

void AppHandle::updateOutput()
{
    {
        juce::ScopedLock sl (outputLock);

        if (output.getDataSize() <= 0)
            return;

        lastOutput = output.toString();
        output.reset();
    }

    listeners.call (&Listener::appOutputAvailable, *this, lastOutput);
}

} // namespace mrlab::controller
