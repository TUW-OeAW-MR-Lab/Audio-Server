/*
    AppControllerTest.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include <catch2/catch_test_macros.hpp>

#include <controller/MainController.h>
#include <controller/AppHandle.h>

namespace mrlab
{

TEST_CASE ("App handle controller", "[AppControllerTest]")
{
    controller::MainController mainController;
    controller::ConfigController& configController = mainController.getConfigController();
    controller::AppController& appController = mainController.getAppController();

    SECTION ("Load and acess example configurations")
    {
        juce::Identifier reaperTest = "reaper_test_mac";
        juce::Identifier pdTest = "pd_test_mac";

        REQUIRE (configController.loadConfig (reaperTest));
        REQUIRE (configController.loadConfig (pdTest));

        REQUIRE (appController.getApps().size() == 2);

        const auto& app = appController.getApp (reaperTest);

        REQUIRE (app.getId() == reaperTest);
        REQUIRE (app.getState() == controller::AppHandle::AppState::initial);
    }
}

} // namespace mrlab
