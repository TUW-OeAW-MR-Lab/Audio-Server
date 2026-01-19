/*
    MainController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include "AppConfigController.h"
#include "AppController.h"
#include "OscController.h"
#include "WebServerController.h"
#include <util/Logger.h>

namespace mrlab::controller
{

//==============================================================================
class MainController
{
public:
    //==============================================================================
    MainController();

    /** @returns a reference to the AppController. */
    AppController& getAppController() { return appController; }

    /** @returns a reference to the OscController. */
    OscController& getOscController() { return oscController; }

    /** @returns a reference to the WebServerController. */
    WebServerController& getWebServerController() { return webServerController; }

private:
    //==============================================================================
    /** Perform a few initialisation tasks/checks. */
    void initialise();

    /** Start the webserver with a default configuration. */
    void startWebServer();

    //==============================================================================
    AppConfigController appConfigController;
    AppController appController;
    OscController oscController;
    WebServerController webServerController;

    Logger logger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainController)
};

} // namespace mrlab::controller
