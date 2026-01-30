/*
    AppController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppController.h"
#include "MainController.h"
#include "AppHandle.h"
#include "YamlConfig.h"
#include <util/Logger.h>

namespace mrlab::controller
{

AppController::AppController (MainController& mainControllerIn)
    : mainController (mainControllerIn)
{
    mainController.getConfigController().addListener (this);
}

AppController::~AppController()
{
    mainController.getConfigController().removeListener (this);
}

bool AppController::add (const YamlConfig& config)
{
    const auto& id = config.getId();

    try
    {
        auto [iter, success] = apps.try_emplace (id, std::make_unique<AppHandle> (config));

        if (success)
            listeners.call (&Listener::appAdded, *iter->second);
        else
            Logger::logWarn ("AppController: App has already been added for id: " + id.toString());

        return success;
    }
    catch (const std::runtime_error& e)
    {
        Logger::logError ("AppController: Error adding app for id: " + id.toString() + "(" + e.what() + ").");
    }

    return false;
}

bool AppController::remove (const juce::Identifier& appId)
{
    checkForAppAndThrowIfNotFound (appId);

    if (apps.at (appId)->isRunning())
    {
        Logger::logWarn ("AppController: Ignoring removal attempt of still running app with id: " + appId.toString());
        jassertfalse;

        return false;
    }

    removeForced (appId);

    return true;
}

void AppController::stopAllApps (std::function<void (bool allFinished)> callWhenDone, uint32_t timeoutMs)
{
    for (auto& [id, app] : apps)
    {
        if (app->isRunning())
            app->stop();
    }

    appStopTimer = std::make_unique<AppStopTimer> (*this, timeoutMs);
    appStopTimer->callWhenDone = std::move (callWhenDone);
}

void AppController::killAllApps()
{
    for (auto& [id, app] : apps)
        app->kill();
}

bool AppController::isAnyAppRunning() const
{
    return std::any_of (apps.begin(), apps.end(), [] (const auto& it) { return it.second->isRunning(); });
}

AppHandle& AppController::getApp (const juce::Identifier& appId)
{
    checkForAppAndThrowIfNotFound (appId);

    return *apps.at (appId);
}

const AppHandle& AppController::getApp (const juce::Identifier& appId) const
{
    checkForAppAndThrowIfNotFound (appId);

    return *apps.at (appId);
}

void AppController::configAdded (const YamlConfig& config)
{
    // Ignore configurations that do not affect us.
    if (! config.hasSection (YamlConfig::Section::app))
        return;

    if (! add (config))
        Logger::logWarn ("AppController: Error adding app for newly appeared config with id: " + config.getId().toString());
}

void AppController::configWillBeRemoved (const YamlConfig& config)
{
    const auto id = config.getId();

    // Ignore configurations that do not affect us.
    if (! config.hasSection (YamlConfig::Section::app))
    {
        // Somehow we ended up managing an app without an app config section?
        jassert (! hasApp (id));
        return;
    }

    if (! hasApp (id))
        return;

    /* Try to be gentle and issue the app's stop command in case it is
       running, but we are actually going to kill it right after.
     */
    auto& app = getApp (id);

    if (app.isRunning())
    {
        Logger::logWarn ("AppController: Config for running app with id '" + id.toString() + "' is going to be removed. Stopping/killing app.");

        app.stop();
    }

    removeForced (id);
}

void AppController::removeForced (const juce::Identifier& appId)
{
    listeners.call (&Listener::appWillBeRemoved, *apps.at (appId));
    apps.erase (appId);
}

void AppController::checkForAppAndThrowIfNotFound (const juce::Identifier& appId) const
{
    if (! apps.contains (appId))
        throw AppUnknownException (appId);
}

AppController::AppStopTimer::AppStopTimer (AppController& appController, uint32_t timeoutMs)
    : controller (appController)
{
    if (timeoutMs > 0)
        numIntervals = timeoutMs / checkIntervalMs + 1;

    startTimer (checkIntervalMs);
}

void AppController::AppStopTimer::timerCallback()
{
    if (! controller.isAnyAppRunning())
    {
        stopTimer();

        if (callWhenDone)
            callWhenDone (false);

        return;
    }

    // Never timeout if numIntervals was already 0.
    if (numIntervals == 0 || --numIntervals > 0)
        return;

    stopTimer();

    if (callWhenDone)
        callWhenDone (true);
}

} // namespace mrlab::controller
