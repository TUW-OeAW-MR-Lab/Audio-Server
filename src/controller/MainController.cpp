/*
    MainController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "MainController.h"
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <iostream>
#include <Globals.h>

namespace mrlab::controller
{

MainController::MainController()
    : appController (configController),
      oscController (*this),
      webServerController (*this)
{
    // Initialisation tasks of this controller.
    initialise();

    // Start web server.
    startWebServer();

    juce::Timer::callAfterDelay (1200, [&] {
        appController.populateFromConfigDir();
    });
}

void MainController::initialise()
{
    // set up logger facility
    Logger::setCurrentLogger (&logger);

    // check for/create application support directory
    const auto appSupportDir = Globals::getAppSupportDir();

    if (! appSupportDir.isDirectory())
    {
        Logger::logWarn (juce::String ("App support dir does not exist, creating it: ") + appSupportDir.getFullPathName());

        const auto result = appSupportDir.createDirectory();

        if (! result)
            Logger::logFatal (juce::String ("Could not create app support dir, error: ") + result.getErrorMessage());
    }
}

void MainController::startWebServer()
{
    if (! webServerController.start())
        Logger::logError ("MainController: Starting the webserver failed!");
}

} // namespace mrlab::controller
