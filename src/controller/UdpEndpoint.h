/*
    UdpEndpoint.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "OscEndpoint.h"
#include <lo/lo_cpp.h>

namespace mrlab::controller
{

//==============================================================================
/** OSC endpoint with UDP transport.

    This uses a separate lo::ServerThread for listening to incoming
    OSC packets. Sending is performed synchronously in the current
    implementation.
 */
class UdpEndpoint : public OscEndpoint
{
public:
    //==============================================================================
    /** Mode how to manage the destination address (e.g., for query responses). */
    enum class DestinationMode
    {
        none          = 0x0,    // No destination, endpoint does not send.
        firstReceived,          // Origin of first received message sets destination (once).
        lastReceived            // Every received message sets destination (reply mode).
    };

    //==============================================================================
    /** Create a new UDP endpoint.

        @param oscControllerIn OscController the endpoint.
        @param listeningPort UDP port to listen to.
        @param mode Determine how to handle sending/replies.

        @throws ServerInvalidException

        @note Listening to incoming data will not start unless start()
              has been called.
     */
    UdpEndpoint (OscController& oscControllerIn, uint16_t listeningPort, DestinationMode mode = DestinationMode::none);

    /** Create a new UDP endpoint setting a remote (destination) address.

        This will set the endpoint to DestinationMode::firstReceived
        (and setting the destination counts as "received"), i.e., the
        given destination address will always be used for sending and
        will not be overwritten by incoming packages from a different
        remote address.

        @param oscControllerIn OscController the endpoint.
        @param listeningPort UDP port to listen to.
        @param destinationIn Remote address to send to.

        @throws ServerInvalidException
        @throws DestinationInvalidException

        @note Listening to incoming data will not start unless start()
              has been called.
     */
    UdpEndpoint (OscController& oscControllerIn, uint16_t listeningPort, const lo::Address& destinationIn);

    ~UdpEndpoint() override;

    /** Start the server thread listening for incoming OSC packets. */
    void start();

protected:
    //==============================================================================
    /** Send an OSC message using this endpoint.

        @param path OSC path of the message.
        @param msg Message object encoding the message arguments.
        @param origin Source endpoint of the message or nullptr if not forwarding.

        @throws SendingOscFailedException.
     */
    void send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin) override;

    /** Called when a message has been received by the endpoint.

        Default behaviour is to simply dispatch the message to the
        OscController. Subclasses may use this, e.g., to filter
        messages or rewrite the path.
     */
    virtual void received (std::string_view path, const lo::Message& msg);

private:
    //==============================================================================
    /** Updates the stored destination address. */
    void updateDestination (const lo::Address& newDestination);

    /** Error handler for lo::ServerThread for logging purposes. */
    void errorHandler (int num, const char* msg, const char* where) const;

    //==============================================================================
    lo::ServerThread server;
    DestinationMode destinationMode { DestinationMode::lastReceived };
    bool shouldUpdateDestination { false };
    std::optional<lo::Address> destination;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UdpEndpoint)
};


/** OSC endpoint using UDP transport and rewriting OSC path functionality.

    This may be used for "mounting" a (partial) client-specific OSC
    address space to a certain point in the global address hierarchy of
    the audio server.

    Path rewriting is configured by setting two parameters, the
    subPath and the prefix:

    Outgoing messages will have
    - subPath stripped from the beginning of their path and subsequently
    - prefix added to the beginning of the resulting path before sending.
    - Additionally, outgoing messages whose path does not begin with subPath
      will be ignored (not sent).

    Incoming messages will have
    - prefix stripped from the beginning of the received path and subsequently
    - subPath added to the beginning of the resulting path before dispatching.
    - Additionally, incoming messages whose path does not begin with prefix
      will be ignored (not dispatched).
 */
class UdpEndpointWithPathRewrite : public UdpEndpoint
{
public:
    //==============================================================================
    /** Create a new UDP endpoint with OSC path rewriting.

        @param oscControllerIn OscController the endpoint.
        @param listeningPort UDP port to listen to.
        @param mode Determine how to handle sending/replies.
        @param subPathIn "Mount point" in the OSC address space of the audio server.
        @param prefixIn Part of the client's OSC address space to be "mounted" (and filtered).

        @throws ServerInvalidException
        @throws OscPathInvalidException

        @note Listening to incoming data will not start unless start()
              has been called.
*/
    UdpEndpointWithPathRewrite (OscController& oscControllerIn, uint16_t listeningPort, DestinationMode mode, std::string_view subPathIn, std::string_view prefixIn = "");

    /** Create a new UDP endpoint with path rewriting and a remote (destination) address.

        This will set the endpoint to DestinationMode::firstReceived
        (and setting the destination counts as "received"), i.e., the
        given destination address will always be used for sending and
        will not be overwritten by incoming packages from a different
        remote address.

        @param oscControllerIn OscController the endpoint.
        @param listeningPort UDP port to listen to.
        @param destinationIn Remote address to send to.
        @param subPathIn "Mount point" in the OSC address space of the audio server.
        @param prefixIn Part of the client's OSC address space to be "mounted" (and filtered).

        @throws ServerInvalidException
        @throws DestinationInvalidException
        @throws OscPathInvalidException

        @note Listening to incoming data will not start unless start()
              has been called.
     */
    UdpEndpointWithPathRewrite (OscController& oscControllerIn, uint16_t listeningPort, const lo::Address& destinationIn, std::string_view subPathIn, std::string_view prefixIn = "");

    /** Get the sub-path for path rewriting.

        @return Sub-path used for path rewriting.
     */
    const std::string& getSubPath() const { return subPath; }

    /** Get the prefix for path rewriting.

        @return Prefix used for path rewriting.
     */
    const std::string& getPrefix() const { return prefix; }

protected:
    //==============================================================================
    /** Send an OSC adress using this endpoint.

        @param path OSC path of the message.
        @param msg Message object encoding the message arguments.
        @param origin Source endpoint of the message or nullptr if not forwarding.

        @throws SendingOscFailedException.
     */
    void send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin) override;

    /** Called when a message has been received by the endpoint. */
    void received (std::string_view path, const lo::Message& msg) override;

    /** Helper to set the subPath and prefix members.

        @note This should not be used after start() has been called
              due to potentially concurrent member access.
     */
    void setSubPathAndPrefix (std::string_view subPathIn, std::string_view prefixIn);

private:
    //==============================================================================
    /** Helper to validate OSC path segments. */
    void validatePathSegment (std::string_view path);

    //==============================================================================
    std::string subPath;
    std::string prefix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UdpEndpointWithPathRewrite)
};

} // namespace mrlab::controller
