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

    // Save current working directory later restore after app launch.
    const auto currentWorkingDir = juce::File::getCurrentWorkingDirectory();

    // Change to configured app working directory if specified (non-empty).
    if (config.workingDir != juce::String() && ! config.workingDir.setAsCurrentWorkingDirectory())
    {
        std::cerr << "AppHandle::start(): Could not set working directory " << config.workingDir.getFullPathName() << std::endl;
        setStateAndNotify (AppState::startFailed);
        return state;
    }

    // Try to launch app.
    const auto success = process.start (config.startCommand, streamFlags);

    // Restore previously saved working directory.
    currentWorkingDir.setAsCurrentWorkingDirectory();

    setStateAndNotify (success ? AppState::alive : AppState::startFailed);

    if (! success)
        return state;

    startTimerHz (stateUpdateHz);

    // Start reading the output asynchronously as juce::ChildProcess::readProcessOutput() is blocking.
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
        return kill();

    juce::ChildProcess stopProcess;

    if (! (stopProcess.start (config.stopCommand) &&
           stopProcess.waitForProcessToFinish (1000) &&
           stopProcess.getExitCode() == 0))
    {
        if (stopProcess.isRunning())
        {
            std::cerr << "AppHandle::stop(): stopCommand is taking too long, killing it..." << std::endl;
            stopProcess.kill();
        }
        else
        {
            std::cerr << "AppHandle::stop(): stopCommand returned " << stopProcess.getExitCode() << ": "
                      << stopProcess.readAllProcessOutput() << std::endl;
        }

        setStateAndNotify (AppState::stopRequestFailed);
        return state;
    }

    setStateAndNotify (AppState::stopRequested);

    return state;
}

AppHandle::AppState AppHandle::kill()
{
    if (process.isRunning())
        setStateAndNotify (process.kill() ? AppState::killRequested : AppState::killRequestFailed);

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
