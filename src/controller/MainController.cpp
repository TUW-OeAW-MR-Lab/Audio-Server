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
#include <util/SupportFileLocation.h>

namespace mrlab::controller
{

MainController::MainController()
    : appController (appConfigController),
      oscController (*this),
      webServerController (*this)
{
    // set up logger facility
    Logger::setCurrentLogger (&logger);

    startWebServer();

    // Simulate dynamic run-time adding after construction time...
    juce::Timer::callAfterDelay (1200, [&] {
        appController.add (AppConfigController::configFly);
        appController.add (AppConfigController::configReverb);
        appController.add (AppConfigController::configJungle);
        appController.add (AppConfigController::configMusic);
    });
}

void MainController::startWebServer()
{
    // TODO: Read Config from central configuration file (IMRV-42)
    WebServerController::Config config;

    config.listenPort = 7080;

    config.documentRoot = APP_SUPPORT_DIR.getChildFile ("webgui");

    Logger::logInfo (juce::String ("WebGUI document root is: ") + config.documentRoot.getFullPathName());

    if (! config.documentRoot.isDirectory())
        Logger::logWarn ("WebGUI document root does not exist!");

    if (! webServerController.start (config))
        Logger::logError ("Starting the webserver failed!");
}

} // namespace mrlab::controller
