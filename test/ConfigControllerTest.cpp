/*
    ConfigControllerTest.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include <catch2/catch_test_macros.hpp>

#include <controller/ConfigController.h>
#include <controller/YamlConfig.h>
#include <Globals.h>
#include <util/Logger.h>

namespace mrlab
{

TEST_CASE ("YAML configuration file controller", "[ConfigControllerTest]")
{
    controller::ConfigController configController;

    SECTION ("Load and access example configurations")
    {
        REQUIRE (configController.loadConfig (Globals::getConfigDir().getChildFile ("reaper_test_mac.yaml")));
        REQUIRE (configController.loadConfig (Globals::getConfigDir().getChildFile ("pd_test_mac.yaml")));

        REQUIRE (configController.getConfigurations().size() == 2);

        REQUIRE_NOTHROW (configController.getConfig (juce::Identifier ("reaper_test_mac")));
        REQUIRE_NOTHROW (configController.getConfig (juce::Identifier ("pd_test_mac")));

        REQUIRE_THROWS_AS (configController.getConfig (juce::Identifier ("non_existent")), controller::ConfigController::ConfigNotFoundException);
    }


    SECTION ("Listener interface when loading and reloading a configuration")
    {
        struct listener : public controller::ConfigController::Listener
        {
            void configAdded (const controller::YamlConfig&) override { ++addedCount; }
            void configWillBeRemoved (const controller::YamlConfig&) override { ++removedCount; }

            int addedCount = 0;
            int removedCount = 0;
        } listener;

        configController.addListener (&listener);

        CHECK (listener.addedCount == 0); REQUIRE (listener.removedCount == 0);

        // load
        CHECK (configController.loadConfig (Globals::getConfigDir().getChildFile ("reaper_test_mac.yaml")));
        CHECK (listener.addedCount == 1); REQUIRE (listener.removedCount == 0);

        // reload
        CHECK (configController.loadConfig ("reaper_test_mac"));
        CHECK (listener.addedCount == 2); REQUIRE (listener.removedCount == 1);

        // unload
        CHECK_NOTHROW (configController.unloadConfig ("reaper_test_mac"));
        CHECK (listener.addedCount == 2); REQUIRE (listener.removedCount == 2);

        configController.removeListener (&listener);
    }


    SECTION ("Load and validate all YAML configuration files from resources")
    {
        // Set up logger facility.
        Logger logger;
        Logger::setCurrentLogger (&logger);

        // Should log failures to test log file.
        configController.populateFromConfigDir();

        const auto numYamlFiles = Globals::getConfigDir().findChildFiles (juce::File::TypesOfFileToFind::findFiles, true, "*." + Globals::getConfigFileExtension()).size();

        REQUIRE (configController.getNumConfigurations() == size_t (numYamlFiles));
    }
}

} // namespace mrlab
