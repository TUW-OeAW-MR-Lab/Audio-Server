/*
    YamlConfig.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "YamlConfig.h"

namespace mrlab::controller
{

YamlConfig::YamlConfig (const juce::File& yamlFile)
    : id (yamlFile.getFileNameWithoutExtension())
{
    load (yamlFile);
    validate();
}

void YamlConfig::load (const juce::File& yamlFile)
{
    node = YAML::LoadFile (yamlFile.getFullPathName().toStdString());
}

void YamlConfig::validate()
{
    // Check top level keys.
    // mandatory: name
    validateMandatoryWithType<std::string> (node["name"], "name", "string");

    // optional: description, author, date
    for (const char* key : { "description", "author", "date" })
        validateOptionalWithDefault (node[key], std::string(), key, "string");

    // optional: revision
    validateOptionalWithDefault (node["revision"], 0, "revision", "int");

    // Validate "app" section if it exists.
    if (hasSection (Section::app))
    {
        auto appRoot = getSection (Section::app);

        // optional: workingDir
        validateOptionalWithDefault (appRoot["workingDir"], std::string(), "app:workingDir", "string");

        // mandatory: startCommand and stopCommand arrays
        for (const char* key : { "startCommand", "stopCommand" })
        {
            const auto& n = appRoot[key];

            if (! (n && n.IsSequence()))
                throw InvalidConfigException (std::string ("Missing or invalid key 'app:") + key + "', must be a sequence of <string>");

            for (const auto& nn : n)
            {
                try
                {
                    nn.as<std::string>();
                }
                catch (const YAML::BadConversion& e)
                {
                    throw InvalidConfigException (std::string ("Invalid key 'app:") + key + "', all sequence members must be of type <string> [" + e.what() + "]");
                }
            }
        }

        // optional: output capture flags
        for (const char* key : { "captureStdOut", "captureStdErr" })
            validateOptionalWithDefault (appRoot[key], false, std::string ("app:") + key, "bool");

        // Validate "oscClients" subsection.
        const auto& oscClients = appRoot["oscClients"];

        if (oscClients)
        {
            if (! oscClients.IsSequence())
                throw InvalidConfigException (std::string ("Invalid key 'app:oscClients', must be a sequence of mappings"));

            // Check oscClients sequence members.
            for (auto i = 0; auto osc : oscClients)
            {
                std::string msgKey = std::string ("app:oscClients[") + std::to_string (i++) + "]";

                if (! osc.IsMap())
                    throw InvalidConfigException (std::string ("Invalid key '" + msgKey + "', all sequence members must be of type <mapping>"));

                // mandatory id, subPath, listenPort
                validateMandatoryWithType<std::string> (osc["id"], msgKey + ":id", "string");
                validateMandatoryWithType<std::string> (osc["subPath"], msgKey + ":subPath", "string");
                validateMandatoryWithType<uint16_t> (osc["listenPort"], msgKey + ":listenPort", "int");

                // optional prefix
                validateOptionalWithDefault (osc["prefix"], std::string(), msgKey + ":prefix", "string");

                // Validate "destination".
                auto destination = osc["destination"];

                if (! (destination && destination.IsSequence() && destination.size() == 2))
                    throw InvalidConfigException (std::string ("Missing or invalid key '" + msgKey + ":destination', must be a sequence of <string, int>"));

                validateMandatoryWithType<std::string> (destination[0], msgKey + ":destination[0]", "string");
                validateMandatoryWithType<uint16_t> (destination[1], msgKey + ":destination[1]", "int");
            }
        }
    }
}

template <std::common_reference_with<YAML::Node> NodeType, typename T>
void YamlConfig::validateOptionalWithDefault (NodeType&& n, T defaultValue, std::string msgKey, std::string msgType)
{
    if (n)
        return validateMandatoryWithType<T> (n, std::move (msgKey), std::move (msgType));

    n = defaultValue;
    return;
}

template <typename T>
void YamlConfig::validateMandatoryWithType (const YAML::Node& n, std::string msgKey, std::string msgType)
{
    if (! n)
        throw InvalidConfigException (std::string ("Missing key '") + msgKey + "' <" + msgType + ">");

    if (! n.IsScalar())
        throw InvalidConfigException (std::string ("Invalid key '") + msgKey + "', must be a scalar <" + msgType + ">");

    try
    {
        n.as<T>();
    }
    catch (const YAML::BadConversion& e)
    {
        throw InvalidConfigException (std::string ("Invalid key '") + msgKey + "', must be of type <" + msgType + "> [" + e.what() + "]");
    }
}

} // namespace mrlab::controller
