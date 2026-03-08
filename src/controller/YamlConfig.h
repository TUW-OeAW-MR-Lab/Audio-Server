/*
    YamlConfig.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include <yaml-cpp/yaml.h>

namespace mrlab::controller
{

//==============================================================================
/** Configuration loaded from a file and represented as a YAML tree.

    A configuration file may contain several sections, e.g., for app or
    routing configurations.
 */
class YamlConfig
{
public:
    //==============================================================================
    /** Configuration section identifiers.

        Allow for testing for and retrieving top-level sections (i.e.,
        top-level map keys) of configuration files.
     */
    enum class Section
    {
        app = 0,                ///< Application section.
        macros                  ///< Control macros section.
    };

    /** Section keys.

        @note On Windows, yaml-cpp apparently does not swallow
              std::map<Section, std::string_view>, hence const char*.
     */
    inline static const auto sectionKeys = std::map<Section, const char*> (
        { { Section::app, "app" },
          { Section::macros, "macros" } }
    );

    //==============================================================================
    /** Create a YamlConfig by reading from a YAML configuration file.

        This will attempt to parse the file into a YAML node and
        validate it against internally stored requirements. If any of
        these steps fails, an exception is thrown and no object is
        constructed, such that any successfully constructed object can
        be assumed to contain a valid configuration.

        @param yamlFile File to load the configuration from.

        @throws ConfigInvalidException on a validation error and any
                of the exceptions thrown by YAML::LoadFile and yaml-cpp
                conversion operations.
     */
    YamlConfig (const juce::File& yamlFile);

    /** Get the id of the configuration.

        By definition, the configuration id is determined by its
        filename (without any path or extension elements).

        @returns The configuration id.
     */
    const juce::Identifier& getId() const { return id; }

    /** @return a reference to the top-level YAML node of the configuration. */
    const YAML::Node& get() const { return node; }

    /** Convenience operator to directly access subnodes of the top-level YAML node. */
    template <typename Key>
    const YAML::Node operator[] (const Key& key) const { return node[key]; }

    /** Retrieve the YAML node of a top-level configuration section.

        @param section The configuration section to retrieve.
        @return The corresponding YAML node (may be undefined in case it is not present).
     */
    const YAML::Node getSection (Section section) const { return node[sectionKeys.at (section)]; }

    /** Check whether the configuration defines the specified top-level section. */
    bool hasSection (Section section) const { return getSection (section).IsDefined(); }

private:
    //==============================================================================
    /** Loads a config from a yaml file while also checking it for validity. */
    void load (const juce::File& yamlFile);

    /** Validates the config. */
    void validate();

    /** Validates an optional node and assigns a default value if none is set.

        If the node is not defined, the provided default value will be
        assigned. If the node is defined, it will be validated using
        the type infered from the default value.

        @tparam NodeType Type of the YAML node passed in (e.g., lvalue/rvalue/reference).
        @tparam T Type to check whether the node can be converted to.
        @param n Node to check.
        @param msgKey Key identifier to be used in exception message.
        @param msgType Type identifier to be used in exception message.
        @throws ConfigInvalidException

        @note This expects the very YAML node to be validated as
              parameter n. The msgKey parameter is not used for
              accessing the node (i.e., from the parent node) but
              merely for user information, as it also might indicate a
              key hierarchy (e.g.,
              "app:oscClients[0]:listenPort"). Similarly, the msgType
              parameter is for user information only, as YAML type
              requirements are most probably not communicated by the
              C++ type names that are used in the respective
              controller-specific implementations.
     */
    template <std::common_reference_with<YAML::Node> NodeType, typename T>
    static void validateOptionalWithDefault (NodeType&& n, T defaultValue, std::string msgKey, std::string msgType);

    /** Validates a node to be a scalar and convertible to a certain type.

        @tparam T Type to check whether the node can be converted to.
        @param n Node to check.
        @param msgKey Key identifier to be used in exception message.
        @param msgType Type identifier to be used in exception message.
        @throws ConfigInvalidException

        @note This expects the very YAML node to be validated as
              parameter n. The msgKey parameter is not used for
              accessing the node (i.e., from the parent node) but
              merely for user information, as it also might indicate a
              key hierarchy (e.g.,
              "app:oscClients[0]:listenPort"). Similarly, the msgType
              parameter is for user information only, as YAML type
              requirements are most probably not communicated by the
              C++ type names that are used in the respective
              controller-specific implementations.
     */
    template <typename T>
    static void validateMandatoryWithType (const YAML::Node& n, std::string msgKey, std::string msgType);

    //==============================================================================
    juce::Identifier id;        ///< Config id.
    YAML::Node node;            ///< Top-level YAML node.
};

} // namespace mrlab::controller
