/*
    UdpEndpoint.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "UdpEndpoint.h"
#include <osc/Util.h>
#include <util/Logger.h>
#include <Exceptions.h>

namespace mrlab::controller
{

UdpEndpoint::UdpEndpoint (OscController& oscControllerIn, uint16_t listeningPort, DestinationMode mode)
    : OscEndpoint (oscControllerIn),
      server (listeningPort, [this] (int num, const char* msg, const char* where) { errorHandler (num, msg, where); }),
      destinationMode (mode),
      shouldUpdateDestination (mode != DestinationMode::none)
{
    if (! server.is_valid())
        throw ServerInvalidException ("Cannot listen to UDP port " + std::to_string (listeningPort));

    // Catch-all handler for message receiving.
    server.add_method (nullptr, nullptr, [this] (std::string_view path, const lo::Message& msg) {
        if (shouldUpdateDestination)
            updateDestination (msg.source());

        received (path, msg);

        return 0; // We are the only handler, so don't look further.
    });
}

UdpEndpoint::UdpEndpoint (OscController& oscControllerIn, uint16_t listeningPort, const lo::Address& destinationIn)
    : UdpEndpoint (oscControllerIn, listeningPort, DestinationMode::firstReceived)
{
    if (! destinationIn.is_valid() || destinationIn.protocol() != LO_UDP)
        throw DestinationInvalidException ("Destination is invalid or has non-UDP protocol: " + destinationIn.url());

    updateDestination (destinationIn);
}

UdpEndpoint::~UdpEndpoint()
{
    if (server.is_valid())
        server.stop();
}

void UdpEndpoint::start()
{
    server.start();
}

void UdpEndpoint::send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin)
{
    // Avoid echoing messages that were dispatched by ourselves.
    if (origin == this)
        return;

    // We send synchronously here as we don't expect blocking delays with UDP.
    if (server.is_valid() && destination.has_value())
    {
        if (destination->send_from (lo_server (server), path, msg) < 0)
            throw SendingOscFailedException ("UdpEndpoint: Sending OSC message failed to " + getPeer());
    }
}

void UdpEndpoint::received (std::string_view path, const lo::Message& msg)
{
    dispatch (path, msg);
}

void UdpEndpoint::updateDestination (const lo::Address& newDestination)
{
    /* Compare address urls and construct new via url rather than
       copying the newDestination object due to uncertain lifetime of
       lo::Address object with internal non-owned flag set (as
       returned by lo::Message::source()), see implementation of
       lo::Address in lo_cpp.cpp.
     */
    if (const auto url = newDestination.url(); ! destination.has_value() || url != destination->url())
    {
        destination.emplace (url);
        setPeer (url);
    }

    shouldUpdateDestination = destinationMode == DestinationMode::lastReceived;
}

void UdpEndpoint::errorHandler (int num, const char* msg, const char* where) const
{
    Logger::logError (osc::Util::formatServerError (server, "UdpEndpoint: ", num, msg, where));
}

//==============================================================================
// UdpEndpointWithPathRewrite implementation from here
//==============================================================================

UdpEndpointWithPathRewrite::UdpEndpointWithPathRewrite (OscController& oscControllerIn,
                                                        uint16_t listeningPort,
                                                        DestinationMode mode,
                                                        std::string_view subPathIn,
                                                        std::string_view prefixIn)
    : UdpEndpoint (oscControllerIn, listeningPort, mode)
{
    setSubPathAndPrefix (subPathIn, prefixIn);
}

UdpEndpointWithPathRewrite::UdpEndpointWithPathRewrite (OscController& oscControllerIn,
                                                        uint16_t listeningPort,
                                                        const lo::Address& destinationIn,
                                                        std::string_view subPathIn,
                                                        std::string_view prefixIn)
    : UdpEndpoint (oscControllerIn, listeningPort, destinationIn)
{
    setSubPathAndPrefix (subPathIn, prefixIn);
}

void UdpEndpointWithPathRewrite::send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin)
{
    // Ignore messages not starting with subPath.
    if (! (subPath.empty() || path.starts_with (subPath)))
        return;

    // Strip subPath and prepend prefix.
    path.remove_prefix (subPath.size());
    auto newPath = prefix;
    newPath += path;

    UdpEndpoint::send (newPath, msg, origin);
}

void UdpEndpointWithPathRewrite::received (std::string_view path, const lo::Message& msg)
{
    // Ignore messages not starting with prefix.
    if (! (prefix.empty() || path.starts_with (prefix)))
        return;

    // Strip prefix and prepend subPath.
    path.remove_prefix (prefix.size());
    auto newPath = subPath;
    newPath += path;

    UdpEndpoint::received (newPath, msg);
}

void UdpEndpointWithPathRewrite::setSubPathAndPrefix (std::string_view subPathIn, std::string_view prefixIn)
{
    validatePathSegment (subPathIn);
    validatePathSegment (prefixIn);

    subPath = subPathIn;
    prefix = prefixIn;
}

void UdpEndpointWithPathRewrite::validatePathSegment (std::string_view path)
{
    if (! osc::Util::validateOscPath (path, true))
        throw OscPathInvalidException (std::string ("UdpEndpointWithPathRewrite: OSC path must have a leading and no trailing /: ") += path);
}

} // namespace mrlab::controller
