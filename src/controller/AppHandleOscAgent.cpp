/*
    AppHandleOscAgent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "AppHandleOscAgent.h"
#include "UdpEndpoint.h"
#include "SubPathOscAgentWithEndpoint.h"
#include <osc/Address.h>
#include <yaml-cpp/yaml.h>

namespace mrlab::controller
{

AppHandleOscAgent::AppHandleOscAgent (OscController& oscControllerIn, AppHandle& appHandleIn)
    : OscAgent (oscControllerIn),
      oscController (oscControllerIn),
      appHandle (appHandleIn),
      oscPathPrefix (std::string (osc::Address::app) += '/')
{
    (oscPathPrefix += appHandle.getId().getCharPointer().getAddress()) += '/';
    oscPathState = oscPathPrefix + "state";
    oscPathExitCode = oscPathPrefix + "exitcode";

    appHandle.addListener (this);
    addMethods();
}

AppHandleOscAgent::~AppHandleOscAgent()
{
    appHandle.removeListener (this);
}

bool AppHandleOscAgent::addOscClientsAgents (const YAML::Node& oscClientsConfig)
{
    if (! oscClientsConfig)
        return true;

    // We only support one client entry for now.
    if (oscClientsConfig.size() > 1)
        Logger::logWarn ("AppHandleOscAgent: Ignoring additional oscClient entries in app config with id " + appHandle.getId().toString());

    const auto client = oscClientsConfig[0];
    const auto listeningPort = client["listenPort"].as<uint16_t>();
    const auto destination = lo::Address (client["destination"][0].as<std::string>(),
                                          client["destination"][1].as<uint16_t>());
    const auto subPath = oscPathPrefix + client["subPath"].as<std::string>();
    const auto prefix = client["prefix"].as<std::string>();

    try
    {
        auto endpoint = std::make_unique<UdpEndpointWithPathRewrite> (oscController,
                                                                      listeningPort,
                                                                      destination,
                                                                      subPath,
                                                                      prefix);
        endpoint->start();

        clientAgent = std::make_unique<SubPathOscAgentWithEndpoint> (oscController,
                                                                     subPath,
                                                                     std::move (endpoint));
    }
    catch (const std::runtime_error& e)
    {
        Logger::logError (juce::String ("AppHandleOscAgent: Could not establish OSC client channel: ") + e.what());
        return false;
    }

    Logger::logInfo (juce::String ("AppHandleOscAgent: Established OSC client channel for app ") + appHandle.getId() +
                      " (listeningPort: " + juce::String (listeningPort) +
                      ", destination: " + destination.url() + ").");

        return true;
}

void AppHandleOscAgent::removeOscClientsAgents()
{
    clientAgent.reset();
}


void AppHandleOscAgent::sendAppState (OscEndpoint* destination)
{
    auto state = appHandle.getState();
    auto msg = osc::Message (int32_t (state), AppHandle::AppStateDescription::get (state));

    sendOrBroadcast (destination, oscPathState, msg);
}

void AppHandleOscAgent::sendAppExitCode (OscEndpoint* destination)
{
    auto msg = osc::Message (int32_t (appHandle.getExitCode()));

    sendOrBroadcast (destination, oscPathExitCode, msg);
}

void AppHandleOscAgent::addMethods()
{
    // Query the app state.
    addMethod (oscPathState, {}, [this] (OscEndpoint* source) {
        sendAppState (source);
    });

    // Query the app exit code.
    addMethod (oscPathExitCode, {}, [this] (OscEndpoint* source) {
        sendAppExitCode (source);
    });

    // App control.
    addMethod (oscPathPrefix + "control", "s", [this] (OscEndpoint* source, lo_arg** argv, int /*argc*/) {
        std::string_view command = &argv[0]->s;

        if (command == "launch")
            appHandle.start();
        else if (command == "quit")
            appHandle.stop();
        else if (command == "kill")
            appHandle.kill();
        else
            sendError (source, osc::Error::appCommandUnknown, command, appHandle.getId().getCharPointer().getAddress());
    });
}

} // namespace mrlab::controller
