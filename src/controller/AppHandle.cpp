/*
    AppHandle.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppHandle.h"
#include "YamlConfig.h"
#include <util/Logger.h>

namespace mrlab::controller
{

AppHandle::AppHandle (const YamlConfig& newConfig)
    : config (newConfig)
{
    if (! config.hasSection (YamlConfig::Section::app))
        throw YamlConfig::UnusableConfigException ("Missing config section 'app'.");
}

AppHandle::~AppHandle()
{
    kill();
    captureThread.reset(); // Might block until app process is gone.
}

AppHandle::AppState AppHandle::start()
{
    if (isRunning() || process.isRunning())
        return state;

    const auto appConfig = config.getSection (YamlConfig::Section::app);

    int streamFlags = (appConfig["captureStdOut"].as<bool>() ? juce::ChildProcess::wantStdOut : 0) |
                      (appConfig["captureStdErr"].as<bool>() ? juce::ChildProcess::wantStdErr : 0);

    // Save current working directory in order to restore it after app launch.
    const auto currentWorkingDir = juce::File::getCurrentWorkingDirectory();

    // Change to configured app working directory (fallback: app support dir).
    const auto workingDir = Globals::getAppSupportDir().getChildFile (appConfig["workingDir"].as<std::string>());

    if (! workingDir.setAsCurrentWorkingDirectory())
    {
        Logger::logError ("AppHandle: Could not set working directory " + workingDir.getFullPathName() + " for app with id: " + config.getId().toString());
        setStateAndNotify (AppState::startFailed);
        return state;
    }

    // Try to launch app.
    juce::StringArray startCommand;

    for (const auto& child : appConfig["startCommand"])
        startCommand.add (child.as<std::string>());

    const auto success = process.start (startCommand, streamFlags);

    // Restore previously saved working directory.
    currentWorkingDir.setAsCurrentWorkingDirectory();

    setStateAndNotify (success ? AppState::alive : AppState::startFailed);

    if (! success)
        return state;

    startTimer (TimerId::stateUpdate, stateUpdateMs);

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

    // TODO: implement waiting for app to become ready (IMRV-33).
    // For now, we just become ready after a delay.
    startTimer (TimerId::appReadyTimeout, appReadyTimeoutMs);

    return state;
}

AppHandle::AppState AppHandle::stop()
{
    if (! isRunning())
        return state;

    juce::StringArray stopCommand;

    for (const auto& child : config.getSection (YamlConfig::Section::app)["stopCommand"])
        stopCommand.add (child.as<std::string>());

    if (stopCommand.isEmpty())
        return kill();

    juce::ChildProcess stopProcess;

    if (! (stopProcess.start (stopCommand) &&
           stopProcess.waitForProcessToFinish (1000) &&
           stopProcess.getExitCode() == 0))
    {
        if (stopProcess.isRunning())
        {
            Logger::logWarn ("AppHandle: stopCommand for app with id " + config.getId().toString() + " is taking too long, killing it.");
            stopProcess.kill();
        }
        else
        {
            Logger::logWarn ("AppHandle: stopCommand for app with id " + config.getId().toString() +
                             " returned with exit code " + juce::String (stopProcess.getExitCode()) +
                             " (" + stopProcess.readAllProcessOutput() + ").");
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

const juce::Identifier& AppHandle::getId() const
{
    return config.getId();
}

uint32_t AppHandle::getExitCode() const
{
    jassert (isFinished());

    return process.getExitCode();
}

void AppHandle::timerCallback (int timerId)
{
    switch (timerId)
    {
        case TimerId::stateUpdate:
            updateState();

            if (! isRunning())
                stopTimer (TimerId::stateUpdate);

            updateOutput();
            break;

        case TimerId::appReadyTimeout:
            stopTimer (TimerId::appReadyTimeout); // Single-shot timer.

            if (state == AppState::alive)
                setStateAndNotify (AppState::ready);

            break;

        default:
            ;
    }
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
