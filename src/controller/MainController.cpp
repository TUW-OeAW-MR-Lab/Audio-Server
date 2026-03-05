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
#include "YamlConfig.h"

namespace mrlab::controller
{

MainController::MainController()
    : webServerController (oscController),
      appController (*this)
{
    // Initialisation tasks of this controller.
    initialise();

    // Start servers and subcontrollers.
    start();
}

MainController::~MainController()
{
    Logger::setCurrentLogger (nullptr);
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

bool MainController::start()
{
    if (! oscController.start())
        return false;

    // Not crucial, may best to do this after OscController was started.
    if (! webServerController.start())
        return false;

    // This must not happen before the OscController was started.
    configController.initOscAgent (oscController);

    return true;
}

} // namespace mrlab::controller
