/*
    AppController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <map>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <util/ListenerInterface.h>
#include "ConfigController.h"

namespace mrlab::controller
{
class MainController;
class AppHandle;

//==============================================================================
class AppController : public ConfigController::Listener
{
public:
    //==============================================================================
    /** Listener interface to get informed about currently managed apps. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Indicates that an app was added to the controller.

            @param app App that was added.
         */
        virtual void appAdded (AppHandle& app) = 0;

        /** Indicates that an app is about to be removed from the controller.

            The corresponding app handle will be deleted after this
            call has returned, thus app will become invalid and should
            only be used within this call.

            @param app App that is going to be removed.
         */
        virtual void appWillBeRemoved (AppHandle& app) = 0;
    };

    //==============================================================================
    AppController (MainController& mainControllerIn);

    ~AppController() override;

    /** Add an app handle for a given config to this controller.

        The provided config needs to contain an app configuration
        section for this to work.

        @param config configuration to use.

        @returns true on success, false if there is already an app
                 with this id or if the provided configuration does
                 not contain an app section.
     */
    bool add (const YamlConfig& config);

    /** Remove the app handle for id from this controller.

        This is only supported for apps that are not running.

        @param appId Unique app identifier.
        @returns true on success, false if the app is in a running state.
        @throws AppUnknownException.
     */
    bool remove (const juce::Identifier& appId);

    /** Request to stop all managed and currently running apps.

        If callWhenDone is not empty, the function will be called with false
        as the timedOut argument once all apps reached a non-running state.
        If timeoutMs is greater than 0, callWhenDone will be called with true
        as the timedOut argument when the time has elapsed and there are still
        apps in a running state.

        @param callWhenDone Function to be called when apps are done or timeout reached.
        @param timeoutMs Timeout in ms after which callWhenDone will be called in any case.

        @note If this is called repeatedly, the callback and the
              running timeout of a previous call will be discarded.
     */
    void stopAllApps (std::function<void (bool timedOut)> callWhenDone, uint32_t timeoutMs = 0);

    /** Request to kill all managed apps unconditionally. */
    void killAllApps();

    /** Query whether there are running apps managed by this.

        @returns false if none of the managed apps are running, true otherwise.
     */
    bool isAnyAppRunning() const;

    /** Check whether this manages an app with the given id. */
    bool hasApp (const juce::Identifier& appId) const { return apps.contains (appId); }

    /** Get the app handle for appId.

        @returns The matching handle for appId.
        @throws AppUnknownException.
     */
    AppHandle& getApp (const juce::Identifier& appId);

    /** Get the const app handle for appId.

        @returns The matching const handle for appId.
        @throws AppUnknownException.
     */
    const AppHandle& getApp (const juce::Identifier& appId) const;

    /** @returns a const reference to the map of managed apps. */
    const std::map<juce::Identifier, std::unique_ptr<AppHandle>>& getApps() { return apps; }

private:
    //==============================================================================
    // ConfigController::Listener interface.
    void configAdded (const YamlConfig& config) override;
    void configWillBeRemoved (const YamlConfig& config) override;

    /** Remove an app handle independent of its running state, possibly killing it. */
    void removeForced (const juce::Identifier& appId);

    /** Check whether we know about an appId and throw an exception otherwise. */
    void checkForAppAndThrowIfNotFound (const juce::Identifier& appId) const;

    //==============================================================================
    /** Helper to be used in stopAllApps(). */
    struct AppStopTimer : juce::Timer
    {
        static constexpr uint32_t checkIntervalMs = 100;

        AppStopTimer (AppController& appController, uint32_t timeoutMs);

        void timerCallback() override;

        AppController& controller;
        uint32_t numIntervals = 0;
        std::function<void (bool allFinished)> callWhenDone;
    };

    //==============================================================================
    MainController& mainController;

    std::map<juce::Identifier, std::unique_ptr<AppHandle>> apps; ///< Managed apps.
    std::unique_ptr<AppStopTimer> appStopTimer;                  ///< Helper used in stopAllApps().

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppController)
};

} // namespace mrlab::controller
