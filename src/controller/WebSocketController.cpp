/*
    WebSocketController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "WebSocketController.h"
#include "OscController.h"
#include <lo/lo_cpp.h>
#include <util/Logger.h>
#include <Exceptions.h>

namespace mrlab::controller
{

WebSocketController::WebSocketController (OscController& oscControllerIn)
    : oscController (oscControllerIn)
{}

WebSocketController::~WebSocketController()
{
    closeAll();
}

void WebSocketController::closeAll()
{
    for (auto& [conn, endpoint] : endpoints)
        endpoint->close();
}

bool WebSocketController::handleConnection (CivetServer*, const mg_connection* conn)
{
    if (! endpoints.contains (conn))
        return true;

    Logger::logError ("WebSocketController: Refusing connection request from already known client " + endpoints.at (conn)->getPeer());

    return false;
}

void WebSocketController::handleReadyState (CivetServer*, mg_connection* conn)
{
    const auto [iter, success] = endpoints.try_emplace (conn, std::make_unique<Endpoint> (oscController, conn));

    if (!success)
        return Logger::logError ("WebSocketController: Successful opening handshake reported for already known client " + iter->second->getPeer() + ".");

    Logger::logInfo ("WebSocketController: Opening handshake succeeded for client " + iter->second->getPeer() + ".");
}

bool WebSocketController::handleData (CivetServer*, mg_connection* conn, int bits, char* data, size_t data_len)
{
    if (! endpoints.contains (conn))
    {
        Logger::logError ("WebSocketController: Data received for unknown client, closing connection.");
        return false;
    }

    return endpoints.at (conn)->handleFrame (bits, std::span ((std::byte*) (data), data_len));
}

void WebSocketController::handleClose (CivetServer*, const mg_connection* conn)
{
    if (! endpoints.contains (conn))
    {
        Logger::logError ("WebSocketController: Close request received for unknown client.");
        return;
    }

    if (endpoints.erase (conn) < 1)
        Logger::logError ("WebSocketController: Error removing client upon close request.");
}

//==============================================================================
// WebSocketController::Endpoint implementation from here.
//==============================================================================

WebSocketController::Endpoint::Endpoint (OscController& oscControllerIn, mg_connection* connIn)
    : OscEndpoint (oscControllerIn),
      conn (connIn)
{
    const auto* info = mg_get_request_info (conn);

    if (info)
        setPeer (std::string (info->remote_addr) + ":" + std::to_string (info->remote_port));
}

WebSocketController::Endpoint::~Endpoint()
{
    // Try to initiate or complete the close handshake.
    close();
}

bool WebSocketController::Endpoint::handleFrame (int bits, std::span<std::byte> frame)
{
    if ((bits & 0xf0) != 0x80)
    {
        // Client sends fragmented data or extension bits set, but we don't support that.
        Logger::logWarn ("WebSocketController::Endpoint: Fragmented or extended protocol data received from " + getPeer() + ".");
        return true;
    }

    const auto opcode = bits & 0x0f;

    if (opcode == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE)
    {
        Logger::logInfo ("WebSocketController::Endpoint: Close request received from " + getPeer() + ".");

        close(); // Acknowledge with sending close frame.
        return false; // Indicate that underlying socket should be closed.
    }

    if (opcode != MG_WEBSOCKET_OPCODE_BINARY)
    {
        // We expect OSC messages to be sent in binary websocket frames.
        Logger::logWarn ("WebSocketController::Endpoint: Non-binary data frame received from " + getPeer() + ".");
        return true;
    };

    dispatch (frame);

    return true;
}

void WebSocketController::Endpoint::close()
{
    // Send close frame only once, according to
    // https://datatracker.ietf.org/doc/html/rfc6455#section-5.5.1
    if (closeSent)
        return;

    constexpr auto opcode = MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE;

    mg_websocket_write (conn, opcode, nullptr, 0);
    closeSent = true;
}

void WebSocketController::Endpoint::send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin)
{
    // Avoid echoing messages that were dispatched by ourselves.
    if (origin == this)
        return;

    // TODO: Send asynchronously to prevent blocking (IMRV-82).

    constexpr auto opcode = MG_WEBSOCKET_OPCODE_BINARY;
    const auto size = msg.length (path);

    std::vector<std::byte> data;
    data.resize (size);
    msg.serialise (path, data.data(), nullptr);

    if (size_t (mg_websocket_write (conn, opcode, (const char*) data.data(), size)) != size)
        throw SendingOscFailedException ("WebSocketController::Endpoint: Sending OSC message failed to " + getPeer());
}

} // namespace mrlab::controller
