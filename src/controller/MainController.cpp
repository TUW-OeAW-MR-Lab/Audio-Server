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

namespace mrlab::controller
{

MainController::MainController()
    : appController (appConfigController),
      webServerController (appController)
{
    startWebServer();

    // Simulate dynamic run-time adding after construction time...
    juce::Timer::callAfterDelay(1200, [&] {
        appController.add (AppConfigController::testConfig0);
        appController.add (AppConfigController::testConfig1);
    });
}

void MainController::startWebServer()
{
    // TODO: Read Config from central configuration file (IMRV-42)
    WebServerController::Config config;

    config.listenPort = 7080;

#if JUCE_WINDOWS
   const auto appSupportDir = juce::File (juce::SystemStats::getEnvironmentVariable ("APPDATA", juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getFullPathName()));
#elif JUCE_MAC
   const auto appSupportDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Application Support");
#elif JUCE_LINUX
   const auto appSupportDir = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".local/share");
#else
   jassertfalse; // Undefined platform.
   const auto appSupportDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
#endif

   config.documentRoot = appSupportDir.getChildFile ("mrlabctrl/webgui");

   std::cout << "INFO: WebGUI document root is: " << config.documentRoot.getFullPathName() << std::endl;

   // TODO: Log errors to log instead of stdout (IMRV-37)
   if (! config.documentRoot.isDirectory())
       std::cerr << "WARNING: WebGUI document root does not exist!" << std::endl;

   if (! webServerController.start (config))
       std::cerr << "ERROR: Starting the webserver failed!" << std::endl;
}

} // namespace mrlab::controller
