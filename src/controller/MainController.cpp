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
#include <Config.h>

namespace mrlab::controller
{

MainController::MainController()
    : appController (appConfigController),
      oscController (*this),
      webServerController (*this)
{
    // Initialisation tasks of this controller.
    initialise();

    // Start web server.
    startWebServer();

    juce::Timer::callAfterDelay (1200, [&] {
        appController.populateFromSceneConfigDir();
    });
}

void MainController::initialise()
{
    // set up logger facility
    Logger::setCurrentLogger (&logger);

    // check for/create application support directory
    const auto appSupportDir = Config::getAppSupportDir();

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
    // TODO: Read Config from central configuration file (IMRV-42)
    const auto documentRoot = Config::getWebServerDocumentRootDir();

    Logger::logInfo (juce::String ("WebGUI document root is: ") + documentRoot.getFullPathName());

    if (! documentRoot.isDirectory())
        Logger::logWarn ("WebGUI document root does not exist!");

    if (! webServerController.start())
        Logger::logError ("Starting the webserver failed!");
}

} // namespace mrlab::controller
