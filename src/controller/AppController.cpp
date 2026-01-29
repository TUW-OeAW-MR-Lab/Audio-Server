/*
    AppController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppController.h"
#include "AppHandle.h"
#include "ConfigController.h"
#include <Globals.h>
#include <util/Logger.h>

namespace mrlab::controller
{

AppController::AppController (ConfigController& newConfigController)
    : configController (newConfigController)
{}

AppController::~AppController()
{}

void AppController::populateFromConfigDir()
{
    const auto configDir = Globals::getConfigDir();

    if (! configDir.isDirectory())
    {
        Logger::logWarn ("AppController: Config directory " + configDir.getFullPathName() + " does not exist.");
        return;
    }

    Logger::logInfo ("AppController: Scanning config directory " + configDir.getFullPathName() + " for YAML configuration files.");
    const auto yamlConfigs = configDir.findChildFiles (juce::File::TypesOfFileToFind::findFiles, true, "*.yaml");

    for (const auto& config : yamlConfigs)
        add (config);
}

bool AppController::add (const juce::Identifier& appId)
{
    auto appConfig = configController.findConfig (appId); // may throw

    auto [iter, success] = apps.try_emplace (appId, std::make_unique<AppHandle> (std::move (appConfig)));

    if (success)
        listeners.call (&Listener::appAdded, *iter->second);

    return success;
}

bool AppController::add (const juce::File& file)
{
    auto appConfig = configController.loadConfigFromFile (file);
    if (! appConfig.has_value())
        return false;

    const auto id = appConfig->id;

    auto [iter, success] = apps.try_emplace (id, std::make_unique<AppHandle> (std::move (*appConfig)));

    if (success)
        listeners.call (&Listener::appAdded, *iter->second);

    return success;
}

bool AppController::remove (const juce::Identifier& appId)
{
    checkForAppAndThrowIfNotFound (appId);

    auto& app = apps.at (appId);

    if (app->isRunning())
    {
        jassertfalse; // App should be checked for a non-running state before attempting to remove it.
        return false;
    }

    listeners.call (&Listener::appWillBeRemoved, *app);

    apps.erase (appId);

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
