/*
    ConfigController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <map>
#include <juce_core/juce_core.h>
#include <util/ListenerInterface.h>

namespace mrlab::controller
{
class YamlConfig;

//==============================================================================
/** Manage and provide configurations from YAML config files. */
class ConfigController
{
public:
    //==============================================================================
    /** Exception that is thrown when there is no config for the given id. */
    class ConfigNotFoundException : public std::runtime_error
    {
    public:
        ConfigNotFoundException (const juce::Identifier& id)
            : std::runtime_error ("ConfigNotFoundException: No config found for id " + id.toString().toStdString())
        {}
    };

    //==============================================================================
    /** Listener interface to get informed about currently managed configs. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Called when a configuration was added (i.e., loaded).

            @param id Id of the added config.
         */
        virtual void configAdded (const YamlConfig& config) = 0;

        /** Called when a configuration is about to be removed.

            This will also be called prior to reloading a configuration.

            @param id Id of the config to be removed.
         */
        virtual void configWillBeRemoved (const YamlConfig& config) = 0;
    };

    //==============================================================================
    ConfigController();
    ~ConfigController();

    /** Get the configuration for id.

        @param id Id of the config to retrieve.
        @returns Reference to the queried configuration.
        @throws ConfigNotFoundException.
     */
    const YamlConfig& getConfig (const juce::Identifier& id) const;

    /** Load a config from a YAML file.

        The id of the YamlConfig is determined by its filename
        (without path or extension).

        @note If a configuration with this id is already known/loaded,
              it will be removed first, so this function can also be
              used for reloading a config file.

        @param yamlFile The config file to (re-) load.

        @return true if the config could be successfully (re-) loaded,
                false on error or exception (will be logged).
     */
    bool loadConfig (const juce::File& yamlFile);

    /** Load a config file by specifying its id.

        By definition, the id of a YamlConfig is determined by its
        filename. Hence, this function will construct a filename from
        the given id and attempt to load a configuration from it. If a
        configuration with this id is already known/loaded, it will be
        removed first, so this function can also be used for reloading
        a config file.

        @param id The config id to (re-) load.

        @return true if the config could be successfully (re-) loaded,
                false on error or exception (will be logged).
     */
    bool loadConfig (const juce::Identifier& id);

    /** Remove the configuration for id.

        @param id Id of the config to unload.
        @throws ConfigNotFoundException.
     */
    void unloadConfig (const juce::Identifier& id);

    /** Read all configurations from the globally set Config dir. */
    void populateFromConfigDir();

    /** @returns the number of managed configurations. */
    size_t getNumConfigurations() { return configs.size(); }

    /** @returns a const reference to the map of managed configurations. */
    const std::map<juce::Identifier, std::unique_ptr<YamlConfig>>& getConfigurations() { return configs; }

private:
    //==============================================================================
    /** Load a config from file.

        @param id The config id to assign.
        @param yamlFile The config file to load.

        @note The id param should match the yamlFile param (filename
              without path and extension).

        @return true if the config could be successfully (re-) loaded,
                false on error or exception (will be logged).
     */
    bool loadConfig (const juce::Identifier& id, const juce::File& yamlFile);

    /** Check whether we know about a config id and throw an exception otherwise. */
    void checkForConfigAndThrowIfNotFound (const juce::Identifier& id) const;

    //==============================================================================
    std::map<juce::Identifier, std::unique_ptr<YamlConfig>> configs; ///< Managed configs.

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigController)
};

} // namespace mrlab::controller
