/*
    AppControllerTest.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include <catch2/catch_test_macros.hpp>

#include <controller/AppController.h>
#include <controller/AppConfigController.h>
#include <controller/AppHandle.h>

namespace mrlab
{

TEST_CASE ("App configurations and handles", "[AppControllerTest]")
{
    controller::AppConfigController appConfigController;

    SECTION ("Access hardcoded example configurations")
    {
        REQUIRE_NOTHROW (appConfigController.findConfig (controller::AppConfigController::configFly));
        REQUIRE_NOTHROW (appConfigController.findConfig (controller::AppConfigController::configReverb));
        REQUIRE_THROWS (appConfigController.findConfig (juce::Identifier ("non-existent")));
    }

    controller::AppController appController (appConfigController);

    SECTION ("Add and access example configurations")
    {
        appController.add (controller::AppConfigController::configFly);
        appController.add (controller::AppConfigController::configReverb);

        REQUIRE (appController.getApps().size() == 2);

        auto& app = appController.getApp (controller::AppConfigController::configFly);

        REQUIRE (app.getConfig().id == controller::AppConfigController::configFly);
        REQUIRE (app.getState() == controller::AppHandle::AppState::initial);
    }
}

} // namespace mrlab
