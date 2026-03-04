/*
    WebSocketController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include <CivetServer.h>
#include "OscEndpoint.h"

namespace mrlab::controller
{
class OscController;

//==============================================================================
/** Controller managing websocket endpoints upon connection or data requests.

    This implements the CivetWebSocketHandler interface to establish
    and close websocket connections and handle data transfer.
 */
class WebSocketController : public CivetWebSocketHandler
{
public:
    //==============================================================================
    WebSocketController (OscController& oscControllerIn);
    ~WebSocketController() override;

    /** Get the number of currently active websocket endpoints.

        @return Number of managed websocket endpoints.
     */
    size_t getNumEndpoints() const { return endpoints.size(); }

    /** Request all endpoints to close their connections.

        This should eventually result in getNumEndpoints() returning
        zero (after a couple of asynchronous operations, not directly
        after closeAll() returns).
     */
    void closeAll();

    // CivetWebSocketHandler interface.
    bool handleConnection (CivetServer* server, const mg_connection* conn) override;
    void handleReadyState (CivetServer* server, mg_connection* conn) override;
    bool handleData (CivetServer* server, mg_connection* conn, int bits, char* data, size_t data_len) override;
    void handleClose (CivetServer* server, const mg_connection* conn) override;

private:
    //==============================================================================
    /** OSC endpoint implementation using websocket transport. */
    class Endpoint : public OscEndpoint
    {
    public:
        //==============================================================================
        /** Create a new websocket endpoint.

            @param oscControllerIn OscController instance.
            @param connIn Civetweb connection handle to use.
         */
        Endpoint (OscController& oscControllerIn, mg_connection* connIn);
        ~Endpoint() override;

        /** Handle a received websocket frame.

            This implementation only supports binary, non-fragmented
            websocket frames according to RFC 6455
            (https://datatracker.ietf.org/doc/html/rfc6455). Other
            frame types are ignored and reported, but not considered
            as an error.

            @param bits Websocketframe header bits and opcode.
            @param frame Binary websocket frame.

            @return true on success, false on remote close request to
                    indicate that the connection should be closed.
         */
        bool handleFrame (int bits, std::span<std::byte> frame);

        /** Initiate a close handshake with the remote peer.

            Eventually, this should result in the destruction of this
            endpoint instance asynchronously by the managing
            WebSocketController instance.
         */
        void close();

    protected:
        //==============================================================================
        /** Send an OSC message using this endpoint.

            @param path OSC path of the message.
            @param msg Message object encoding the message arguments.
            @param origin Source endpoint of the message or nullptr if not forwarding.

            @throws SendingOscFailedException.
     */
        void send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin) override;

    private:
        //==============================================================================
        mg_connection* conn;      ///< Civetweb connection handler for this endpoint.
        bool closeSent { false }; ///< Flag whether a close frame has been sent.
    };

    //==============================================================================
    OscController& oscController;

    /// Currently established client connections.
    std::map<const mg_connection*, std::unique_ptr<Endpoint>> endpoints;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebSocketController)
};

} // namespace mrlab::controller
