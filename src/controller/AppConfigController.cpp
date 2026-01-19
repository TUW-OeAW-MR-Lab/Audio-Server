/*
    AppConfigController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppConfigController.h"
#include <util/Logger.h>

namespace mrlab::controller
{

AppConfigController::AppConfigController()
{}

AppConfig AppConfigController::findConfig (const juce::Identifier& appId) const
{
    if (appId == configFly)
    {
        return {
            .id = configFly,
            .name = "Pd Fly",
            .description = "Pd with Fly demonstration",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\PD\\YAMI\\FLY.bat"),
            .stopCommand = juce::StringArray ("C:\\Windows\\System32\\taskkill", "/IM", "pd.com", "/T", "/F"),
            .workingDir = juce::File ("C:\\PD\\YAMI")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/Pd-0.56-2.app/Contents/MacOS/Pd",
                                               "/Users/rm/Documents/Pd/test_stdout.pd"),
            .stopCommand = juce::StringArray ("killall", "Pd"),
            .workingDir = juce::File ("/Users/rm/Documents/Pd")
#endif
        };
    }

    if (appId == configReverb)
    {
        return {
            .id = configReverb,
            .name = "Reaper Reverb",
            .description = "Reaper DAW with artificial reverb",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\Program Files\\REAPER (x64)\\reaper",
                                               "reverb.rpp")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/REAPER.app/Contents/MacOS/REAPER",
                                               "-new")
#endif
        };
    }

    if (appId == configJungle)
    {
        return {
            .id = configJungle,
            .name = "Pd Jungle",
            .description = "Pd with Jungle demonstration",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\PD\\YAMI\\JUNGLE.bat"),
            .stopCommand = juce::StringArray ("C:\\Windows\\System32\\taskkill", "/IM", "pd.com", "/T", "/F"),
            .workingDir = juce::File ("C:\\PD\\YAMI")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/Pd-0.56-2.app/Contents/MacOS/Pd",
                                               "/Users/rm/Documents/Pd/test_stdout.pd"),
            .stopCommand = juce::StringArray ("killall", "Pd")
#endif
        };
    }

    if (appId == configMusic)
    {
        return {
            .id = configMusic,
            .name = "Reaper Music",
            .description = "Reaper DAW with music example",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\Program Files\\REAPER (x64)\\reaper",
                                               "music.rpp")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/REAPER.app/Contents/MacOS/REAPER",
                                               "-new")
#endif
        };
    }

    throw AppConfigNotFoundException (appId);
}

std::optional<AppConfig> AppConfigController::loadConfigFromFile (const juce::File& yamlFile)
{
    Logger::logInfo (juce::String ("Loading config file ") + yamlFile.getFullPathName());

    YamlDocument doc;

    if (! parseYamlFile (yamlFile, doc))
    {
        Logger::logError ("Failed to parse YAML file");
        return std::nullopt;
    }

    if (! validateYamlDocument (doc))
    {
        Logger::logError ("Failed to validate YAML file");
        return std::nullopt;
    }

    AppConfig cfg;
    auto appRoot = doc.node["app"];
    auto root = doc.node;

    try
    {
        // Required strings
        cfg.id = juce::Identifier (yamlFile.getFileNameWithoutExtension());
        cfg.name = root["name"].as<std::string>();
        cfg.description = root["description"].as<std::string>();

        cfg.workingDir = juce::File (appRoot["workingDir"].as<std::string>());

        // Arrays
        cfg.startCommand.clear();
        for (const auto& child : appRoot["startCommand"])
            cfg.startCommand.add (child.as<std::string>());

        cfg.stopCommand.clear();
        for (const auto& child : appRoot["stopCommand"])
            cfg.stopCommand.add (child.as<std::string>());

        // Booleans (optional, default true)
        if (appRoot["captureStdOut"])
            cfg.captureStdOut = (appRoot["captureStdOut"].as<bool>());
        if (appRoot["captureStdErr"])
            cfg.captureStdErr = (appRoot["captureStdErr"].as<bool>());
    }
    catch (const std::exception& e)
    {
        Logger::logError (juce::String ("Error loading AppConfig: ") + e.what());
        return std::nullopt;
    }

    Logger::logInfo ("Config file loaded successfully!");
    return cfg;
}

bool AppConfigController::parseYamlFile (const juce::File& yamlFile, YamlDocument& document)
{
    if (! yamlFile.existsAsFile())
    {
        Logger::logError (juce::String ("AppConfigController::parseYamlFile(): Config file not found."));
        return false;
    }

    try
    {
        document.node = YAML::LoadFile (yamlFile.getFullPathName().toStdString());
    }
    catch (const std::exception& e)
    {
        Logger::logError (juce::String ("AppConfigController::parseYamlFile(): Failed to parse YAML file with error: ") + e.what());
        return false;
    }

    return true;
}

bool AppConfigController::validateYamlDocument (const YamlDocument& document)
{
    // check top level contents
    const auto root = document.node;

    for (const char* key : { "name", "description" })
    {
        if (! root[key])
        {
            Logger::logError (juce::String ("Missing key '") + key + "'");
            return false;
        }
    }

    // check "app" if it exists
    if (document.node["app"])
    {
        const auto appRoot = document.node["app"];

        // Check required string entries
        for (const char* key : { "workingDir" })
        {
            if (! appRoot[key])
            {
                Logger::logError (juce::String ("Missing key '") + key + "'");
                return false;
            }
        }

        // Check startCommand and stopCommand arrays
        for (const char* key : { "startCommand", "stopCommand" })
        {
            if (! appRoot[key] || ! appRoot[key].IsSequence())
            {
                Logger::logError (juce::String ("Missing or invalid '") + key + "' (must be a sequence)");
                return false;
            }
        }
    }

    return true;
}

} // namespace mrlab::controller
