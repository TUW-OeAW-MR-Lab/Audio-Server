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
        REQUIRE_NOTHROW (appConfigController.findConfig (controller::AppConfigController::testConfig0));
        REQUIRE_NOTHROW (appConfigController.findConfig (controller::AppConfigController::testConfig1));
        REQUIRE_THROWS (appConfigController.findConfig (juce::Identifier ("non-existent")));
    }

    controller::AppController appController (appConfigController);

    SECTION ("Add and access example configurations")
    {
        appController.add (controller::AppConfigController::testConfig0);
        appController.add (controller::AppConfigController::testConfig1);

        REQUIRE (appController.getApps().size() == 2);

        auto& app = appController.getApp (controller::AppConfigController::testConfig0);

        REQUIRE (app.getConfig().id == controller::AppConfigController::testConfig0);
        REQUIRE (app.getState() == controller::AppHandle::AppState::initial);
    }
}

} // namespace mrlab
