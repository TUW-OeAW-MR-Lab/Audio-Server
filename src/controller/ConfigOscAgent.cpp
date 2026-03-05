/*
    ConfigOscAgent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "ConfigOscAgent.h"
#include <osc/Address.h>
#include <osc/Util.h>

namespace mrlab::controller
{

ConfigOscAgent::ConfigOscAgent (OscController& oscControllerIn, ConfigController& configControllerIn)
    : OscAgent (oscControllerIn),
      configController (configControllerIn)
{
    configController.addListener (this);
    addMethods();
}

ConfigOscAgent::~ConfigOscAgent()
{
    configController.removeListener (this);
}

void ConfigOscAgent::configAdded (const YamlConfig& config)
{
    sendConfiginfoNum();
    sendConfiginfoList();

    // TODO: Implement per-config osc query interface (IMRV-80).
    juce::ignoreUnused (config);
}

void ConfigOscAgent::configWillBeRemoved (const YamlConfig& config)
{
    // TODO: Implement per-config osc query interface (IMRV-80).
    juce::ignoreUnused (config);
}

void ConfigOscAgent::configHasBeenRemoved (const juce::Identifier& /*id*/)
{
    sendConfiginfoNum();
    sendConfiginfoList();
}

void ConfigOscAgent::sendConfiginfoList (OscEndpoint* destination)
{
    auto msg = lo::Message();

    for (const auto& [id, yaml] : configController.getConfigurations())
        msg.add_string (id.getCharPointer().getAddress());

    sendOrBroadcast (destination, osc::Address::configinfo_list, msg);
}

void ConfigOscAgent::sendConfiginfoNum (OscEndpoint* destination)
{
    auto msg = osc::Message (int32_t (configController.getConfigurations().size()));

    sendOrBroadcast (destination, osc::Address::configinfo_num, msg);
}

void ConfigOscAgent::addMethods()
{
    // Query the config list.
    addMethod (osc::Address::configinfo_list, "", [this] (OscEndpoint* source) {
        sendConfiginfoList (source);
    });

    // Query the number of configs.
    addMethod (osc::Address::configinfo_num, "", [this] (OscEndpoint* source) {
        sendConfiginfoNum (source);
    });

    // Config control, currently only reloading.
    addMethod ("/config/*/control", "s", [this] (OscEndpoint* source, std::string_view path, std::string_view /*types*/, lo_arg** argv, int /*argc*/) {
        // Get matched wildcard portion of OSC path, i.e., the config id.
        const auto idsv = osc::Util::getPathSegment (path, 1);
        const auto id = juce::Identifier (juce::CharPointer_UTF8 (idsv.data()), juce::CharPointer_UTF8 (idsv.data() + idsv.size()));

        if (! configController.hasConfig (id))
            return sendError (source, osc::Error::configIdUnknown, idsv);

        std::string_view command = &argv[0]->s;

        if (command != "reload")
            return sendError (source, osc::Error::configCommandUnknown, command, idsv);

        const auto result = configController.loadConfig (id);

        if (! result)
            sendError (source, osc::Error::configReloadFailed, idsv);
    });
}

} // namespace mrlab::controller
