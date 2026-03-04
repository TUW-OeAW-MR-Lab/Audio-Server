/*
    AppHandleOscAgent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "AppHandle.h"
#include "OscAgent.h"

namespace YAML
{
class Node;
}

namespace mrlab::controller
{
class SubPathOscAgentWithEndpoint;

//==============================================================================
/** OSC agent for AppHandle.

    Implements the OSC interface for app control (e.g.,
    starting/stopping) and state retrieval and manages optional client
    channel handlers.
 */
class AppHandleOscAgent : public OscAgent,
                          public AppHandle::Listener
{
public:
    //==============================================================================
    AppHandleOscAgent (OscController& oscControllerIn, AppHandle& appHandleIn);
    ~AppHandleOscAgent() override;

    // AppHandle::Listener interface
    void appStateChanged (AppHandle&, AppHandle::AppState) override { sendAppState(); }
    void exitCodeAvailable (AppHandle&, uint32_t) override { sendAppExitCode(); }

    /** Add OSC client channel agent(s) according to config section.

        @param oscClientsConfig Validated YamlConfig oscClients: section.
        @return true on success, false on error.

        @note The current implementation only supports one osc client
              channel per configuration.
     */
    bool addOscClientsAgents (const YAML::Node& oscClientsConfig);

    /** Remove all previously added client channel agents. */
    void removeOscClientsAgents();

private:
    //==============================================================================
    /** OSC message sending helper for /app/<id>/state. */
    void sendAppState (OscEndpoint* destination = nullptr);

    /** OSC message sending helper for /app/<id>/exitcode. */
    void sendAppExitCode (OscEndpoint* destination = nullptr);

    /** Register OSC message handlers. */
    void addMethods();

    //==============================================================================
    OscController& oscController; // Reference to OscController.
    AppHandle& appHandle;         // AppHandle for which we do the OSC stuff.
    std::string oscPathPrefix;    // OSC path prefix for this app.
    std::string oscPathState;     // OSC path of state message for this app.
    std::string oscPathExitCode;  // OSC path of exit code message for this app.
    std::unique_ptr<SubPathOscAgentWithEndpoint> clientAgent; // Client agent, if configured.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppHandleOscAgent)
};

} // namespace mrlab::controller
