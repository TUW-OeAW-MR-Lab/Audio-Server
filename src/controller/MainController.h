/*
    MainController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include "AppController.h"
#include "ConfigController.h"
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
    ~MainController();

    /** @returns a reference to the OscController. */
    OscController& getOscController() { return oscController; }

    /** @returns a reference to the WebServerController. */
    WebServerController& getWebServerController() { return webServerController; }

    /** @returns a reference to the ConfigController. */
    ConfigController& getConfigController() { return configController; }

    /** @returns a reference to the AppController. */
    AppController& getAppController() { return appController; }

    /** @returns a reference to the Logger. */
    Logger& getLogger() { return logger; }

private:
    //==============================================================================
    /** Perform a few initialisation tasks/checks. */
    void initialise();

    /** Start managed servers. */
    bool start();

    //==============================================================================
    Logger logger;

    OscController oscController;
    WebServerController webServerController;
    ConfigController configController;
    AppController appController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainController)
};

} // namespace mrlab::controller
