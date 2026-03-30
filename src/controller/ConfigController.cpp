/*
    ConfigController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "ConfigController.h"
#include "ConfigOscAgent.h"
#include "YamlConfig.h"
#include <Globals.h>
#include <util/Logger.h>
#include <Exceptions.h>

namespace mrlab::controller
{

ConfigController::ConfigController() {}

ConfigController::~ConfigController() {}

const YamlConfig& ConfigController::getConfig (const juce::Identifier& id) const
{
    checkForConfigAndThrowIfNotFound (id);

    return *configs.at (id);
}

void ConfigController::initOscAgent (OscController& oscController)
{
    oscAgent = std::make_unique<ConfigOscAgent> (oscController, *this);
}

bool ConfigController::loadConfig (const juce::File& yamlFile)
{
    return loadConfig (yamlFile.getFileNameWithoutExtension(), yamlFile);
}

bool ConfigController::loadConfig (const juce::Identifier& id)
{
    const auto file = Globals::getConfigDir().getChildFile (id.toString()).withFileExtension (Globals::getConfigFileExtension());

    return loadConfig (id, file);
}

void ConfigController::unloadConfig (const juce::Identifier& id)
{
    checkForConfigAndThrowIfNotFound (id);

    Logger::logInfo ("ConfigController: Unloading config with id: " + id.toString());

    listeners.call (&Listener::configWillBeRemoved, *configs.at (id));
    configs.erase (id);
    listeners.call (&Listener::configHasBeenRemoved, id);
}

void ConfigController::unloadAllConfigurations()
{
    while (! configs.empty())
        unloadConfig (configs.begin()->first);
}


void ConfigController::populateFromConfigDir()
{
    const auto configDir = Globals::getConfigDir();

    if (! configDir.isDirectory())
    {
        Logger::logWarn ("ConfigController: Config directory does not exist: " + configDir.getFullPathName());
        return;
    }

    Logger::logInfo ("ConfigController: Scanning config directory for YAML files: " + configDir.getFullPathName());

    const auto yamlFiles = configDir.findChildFiles (juce::File::TypesOfFileToFind::findFiles, true, "*." + Globals::getConfigFileExtension());
    auto result = true;

    for (const auto& yamlFile : yamlFiles)
        result &= loadConfig (yamlFile);

    if (! result)
        Logger::logWarn ("ConfigController: At least one config file could not be loaded successfully.");
}

bool ConfigController::loadConfig (const juce::Identifier& id, const juce::File& yamlFile)
{
    // id and file name have to match.
    jassert (id.toString() == yamlFile.getFileNameWithoutExtension());

    if (configs.contains (id))
        unloadConfig (id);

    try
    {
        auto [iter, success] = configs.try_emplace (id, std::make_unique<YamlConfig> (yamlFile));

        if (success)
        {
            listeners.call (&Listener::configAdded, *iter->second);
            Logger::logInfo ("ConfigController: Loaded config with id: " + id.toString());
        }
        else
        {
            Logger::logError ("ConfigController: Error adding config with id: " + id.toString());
        }

        return success;
    }
    catch (const std::runtime_error& e)
    {
        Logger::logError ("ConfigController: Error loading config with id: " + id.toString() + " (" + e.what() + ").");
    }

    return false;
}

void ConfigController::checkForConfigAndThrowIfNotFound (const juce::Identifier& id) const
{
    if (! configs.contains (id))
        throw ConfigNotFoundException (id);
}

} // namespace mrlab::controller
