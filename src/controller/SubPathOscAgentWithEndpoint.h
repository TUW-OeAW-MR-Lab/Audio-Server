/*
    SubPathOscAgentWithEndpoint.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "OscAgent.h"

namespace mrlab::controller
{

//==============================================================================
/** OSC agent with an endpoint handling a sub-path of the OSC address space.

    Wildcard-listens to all messages that are dispatched below an
    entire sub-path of the OSC address space and forwards them to/from
    the endpoint according to the configured policy.
 */
class SubPathOscAgentWithEndpoint : public OscAgent
{
public:
    //==============================================================================
    /** Forwarding policy for OSC messages dispatched under sub-path. */
    enum class ForwardingPolicy
    {
        broadcast = 0x0, ///< Broadcast all messages.
        endpointOnly     ///< Forward messages not originating from endpoint to endpoint only.
    };

    //==============================================================================
    /** Create a sub-path handling OSC agent.

        @param oscControllerIn Reference to the OscController.
        @param subPathIn The OSC sub-path to listen to.
        @param endpointIn OscEndpoint to manage.
        @param policyIn Message forwarding policy to employ.

        @throws OscPathInvalidException
     */
    SubPathOscAgentWithEndpoint (OscController& oscControllerIn,
                                 std::string_view subPathIn,
                                 std::unique_ptr<OscEndpoint>&& endpointIn,
                                 ForwardingPolicy policyIn = ForwardingPolicy::endpointOnly);
    ~SubPathOscAgentWithEndpoint() override;

private:
    //==============================================================================
    std::unique_ptr<OscEndpoint> endpoint; // Managed endpoint.
    ForwardingPolicy policy;               // Message forwarding policy.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SubPathOscAgentWithEndpoint)
};

} // namespace mrlab::controller
