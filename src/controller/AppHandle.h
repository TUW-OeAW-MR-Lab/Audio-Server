/*
    AppHandle.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <util/ListenerInterface.h>
#include <thread>
#include "AppConfigController.h"

namespace mrlab::controller
{

//==============================================================================
class AppHandle : private juce::Timer
{
public:
    //==============================================================================
    /** App state flags.

        The conditions for state changes are partially app-specific, corresponding
        to definitions in the respective AppConfig (e.g., how to stop an app or how
        the ready, busy, lost and crash states are determined).
     */
    enum class AppState
    {
        // Bitmasks for state classes
        initial         = 0x10,         ///< Process was not yet started.
        finished        = 0x20,         ///< Process stopped in a regular way.
        aborted         = 0x40,         ///< Process was killed, crashed or failed to start.
        running         = 0x80,         ///< Process is running.

        // Single states from here
        init            = initial,      ///< Initial app state without any transitions so far.
        startFailed,                    ///< App failed to start (e.g., the executable was not found).

        stoppedSuccess  = finished,     ///< App has stopped successfully (return code 0).
        stoppedError,                   ///< App has stopped indicating an error.

        crashed         = aborted,      ///< App is considered to have crashed.
        killed,                         ///< App has been successfully killed.

        alive           = running,      ///< App process is alive but app logic not yet considered ready.
        ready,                          ///< App is ready to be used/controlled.
        busy,                           ///< App indicates that it is not ready to be controlled.
        lost,                           ///< Connection to the app was lost.

        stopRequested,                  ///< App has been requested to stop gracefully.
        killRequested,                  ///< System was requested to kill the app.
        killFailed                      ///< Attempt to kill the app failed.
    };

    /** AppState values as display strings. */
    inline static const auto appStateNames = std::map<AppState, juce::String> (
        { { AppState::init, "" },
          { AppState::startFailed, "start failed" },
          { AppState::stoppedSuccess, "success" },
          { AppState::stoppedError, "error" },
          { AppState::crashed, "crashed" },
          { AppState::killed, "killed" },
          { AppState::alive, "alive" },
          { AppState::ready, "ready" },
          { AppState::busy, "busy" },
          { AppState::lost, "lost" },
          { AppState::stopRequested, "quitting..." },
          { AppState::killRequested, "killing..." },
          { AppState::killFailed, "kill failed" } }
    );

    //==============================================================================
    /** Listener interface to get informed of AppHandle updates. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Indicates that the app's state changed.

            @param app App whose state changed.
            @param newState New state of the app.
         */
        virtual void appStateChanged (AppHandle& app, AppState newState) = 0;

        /** Indicates that new output was captured from the app's standard and/or error output.

            @param app App whose output has been captured.
            @param newOutput Newly captured output.

            @note Whenever this is called, previously captured output will have been discarded
                  and also getOutput() will only return newOutput.
         */
        virtual void appOutputAvailable (AppHandle& app, const juce::String& newOutput) = 0;
    };

    //==============================================================================
    /** Create an AppHandle according to an app config.

        @param config App config to use.
     */
    AppHandle (AppConfig newConfig);

    /** Attempt to start this app according to the configuration.

        If the app is already in a running state, this will do nothing but
        returning its current state.

        @returns The AppState after starting, i.e., on success in most cases
                 AppState::alive (but depending on the configuration),
                 AppState::startFailed on error (e.g., executable not found).
     */
    AppState start();

    /** Request to gracefully stop this app.

        If the app is already in AppState::stopRequested, the stop request
        will be repeated according to the configuration.

        If the app is not in a running state, this will do nothing but
        returning its current state.

        @returns The AppState after the stopping request, i.e., in most cases
                 AppState::stopRequested or, depending on the configuration,
                 AppState::stoppedSuccess or another finished state.
     */
    AppState stop();

    /** Attempt to unconditionally kill (abort) the app.

        If the app process is not running, this will do nothing but
        returning its current state.

        @returns AppState::killed on success, AppState::killFailed on failure,
                 or the previous AppState if process was not running.
     */
    AppState kill();

    /** Get the configuration of this app.

        @returns The app configuration.
    */
    const AppConfig& getConfig() const { return config; }

    /** Get the current state of this app.

        @returns The current AppState.
     */
    AppState getState() const { return state; }

    /** Get the exit code of this app if it finished execution.

        @returns The exit code of the specified app.

        @note Calling this does only make sense if the app is not running anymore.
     */
    uint32_t getExitCode() const;

    /** Get the output that was last captured from the app.

        When the app produces continuous output, this will be updated regularly.
        It is recommend to use the AppHandle::Listener interface to get informed
        of output updates.

        @returns A reference to a string containing the app's standard and/or error output.
     */
    const juce::String& getOutput() const { return lastOutput; }

    /** Convenience function to check whether this app is in a running state.

        @returns true if in a running state, false otherwise.
     */
    bool isRunning() const { return int (state) & int (AppState::running); }

    /** Convenience function to check whether this app is in a finished state.

        @returns true if in a finished state, false otherwise.
     */
    bool isFinished() const { return int (state) & int (AppState::finished); }

private:
    //==============================================================================
    /** Rate at which to update the process state and to read the process output. */
    static constexpr auto stateUpdateHz = 10;

    //==============================================================================
    void timerCallback() override;

    /** Sets the app state and notifies listeners. */
    void setStateAndNotify (AppState newState);

    /** Check whether the process is still running and update state. */
    void updateState();

    /** Try to read output from the process. */
    void updateOutput();

    //==============================================================================
    AppConfig config;                            ///< App configuration.
    juce::ChildProcess process;                  ///< Actual app process.
    AppState state = AppState::init;             ///< Current app state.
    juce::String lastOutput;                     ///< Last available output captured from the app.
    juce::MemoryOutputStream output;             ///< Output currently being captured.
    juce::CriticalSection outputLock;            ///< Lock for concurrent access to output.
    std::unique_ptr<std::jthread> captureThread; ///< Thread for asynchronous output capturing.

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppHandle)
};

} // namespace mrlab::controller
