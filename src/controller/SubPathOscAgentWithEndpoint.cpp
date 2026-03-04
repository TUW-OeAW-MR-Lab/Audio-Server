/*
    SubPathOscAgentWithEndpoint.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "SubPathOscAgentWithEndpoint.h"
#include <osc/Util.h>
#include <Exceptions.h>

namespace mrlab::controller
{

SubPathOscAgentWithEndpoint::SubPathOscAgentWithEndpoint (OscController& oscControllerIn,
                                                          std::string_view subPath,
                                                          std::unique_ptr<OscEndpoint>&& endpointIn,
                                                          ForwardingPolicy policyIn)
    : OscAgent (oscControllerIn),
      endpoint (std::move (endpointIn)),
      policy (policyIn)
{
    if (! osc::Util::validateOscPath (subPath))
        throw OscPathInvalidException (std::string ("SubPathWithOscAgentWithEndpoint: subPathIn must have a leading and no trailing /: ") += subPath);

    addMethod (std::string (subPath) += "/*", [this] (OscEndpoint* source, std::string_view path, const lo::Message& msg) {
        if (policy == ForwardingPolicy::endpointOnly && source != endpoint.get())
            return send (*endpoint, path, msg, source);

        broadcast (path, msg, source);
    });
}

SubPathOscAgentWithEndpoint::~SubPathOscAgentWithEndpoint()
{}

} // namespace mrlab::controller
