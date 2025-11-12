/*
    WebServerController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "WebServerController.h"
#include "AppConfigController.h"
#include "MainController.h"
#include "OscController.h"
#include <iostream>
#include <lo/lo_cpp.h>

namespace mrlab::controller
{

WebServerController::WebServerController (MainController& mainControllerIn)
    : mainController (mainControllerIn)
{
    mg_init_library (0);

    mainController.getAppController().addListener (this);
}

WebServerController::~WebServerController()
{
    mainController.getAppController().removeListener (this);

    stop();
    mg_exit_library();
}

bool WebServerController::start (const Config& config)
{
    if (civetServer != nullptr)
        return false;

    std::vector<std::string> civetOptions;

    civetOptions.push_back ("document_root");
    civetOptions.push_back (config.documentRoot.getFullPathName().toStdString());
    civetOptions.push_back ("listening_ports");
    civetOptions.push_back (std::to_string (config.listenPort));
    civetOptions.push_back ("websocket_timeout_ms");
    civetOptions.push_back (std::to_string (5000));
    civetOptions.push_back ("enable_websocket_ping_pong");
    civetOptions.push_back ("yes");

    civetServer = std::make_unique<CivetServer> (civetOptions);

    if (civetServer->getContext() == nullptr)
    {
        civetServer.reset();
        return false;
    }

    // TODO: Add proper error handling/reporting (e.g. port is not available) (IMRV-40).

    civetServer->addWebSocketHandler ("/ws", this);

    return true;
}

bool WebServerController::stop()
{
    if (civetServer == nullptr)
        return false;

    civetServer.reset();

    return true;
}

bool WebServerController::sendToAll (const std::vector<std::byte>& message)
{
    const auto opcode = MG_WEBSOCKET_OPCODE_BINARY;
    auto success = true;

    for (auto* client : clients)
    {
        auto result = size_t (mg_websocket_write (client, opcode, (const char*) message.data(), message.size()));
        success &= result == message.size();
    }

    return success;
}

bool WebServerController::handleConnection (CivetServer* server, const mg_connection* conn)
{
    juce::ignoreUnused (conn);

    jassert (server == civetServer.get());

    std::cout << "WebSocket connection request" << std::endl;

    return true;
}

void WebServerController::handleReadyState (CivetServer* server, mg_connection* conn)
{
    jassert (server == civetServer.get());
    jassert (std::find (clients.begin(), clients.end(), conn) == clients.end()); // We already now this client?

    clients.push_back (conn);

    std::cout << "WebSocket connection ready" << std::endl;
}

bool WebServerController::handleData (CivetServer* server, mg_connection* conn, int bits, char* data, size_t data_len)
{
    juce::ignoreUnused (server, conn);

    jassert ((bits & 0xf0) == 0x80); // Client sends fragmented data or extension bits set, but we don't support that.

    const auto opcode = bits & 0x0f;

    if (opcode == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE)
    {
        std::cout << "WebSocket connection close request received: " << data << std::endl;
        return false;
    }

    jassert (opcode == MG_WEBSOCKET_OPCODE_BINARY); // We expect OSC messages to be sent in binary websocket frames.

    // when receiving OSC, the address will be zero-padded in any case.
    std::cout << "WebSocket frame received: " << data << std::endl;

    mainController.getOscController().dispatchRaw (std::span ((std::byte*) (data), data_len));

    return true;
}

void WebServerController::handleClose (CivetServer* server, const mg_connection* conn)
{
    jassert (server == civetServer.get());

    auto num = std::erase (clients, conn);

    jassertquiet (num > 0); // We don't know this client!

    std::cout << "WebSocket connection closed" << std::endl;
}

void WebServerController::appStateChanged (AppHandle& app, AppHandle::AppState newState)
{
    lo::Message message;
    message.add (int (newState));
    message.add_string (AppHandle::appStateNames.at (newState).toStdString());

    auto oscPath = std::string ("/app/") + app.getConfig().id.toString().toStdString() + "/state";
    auto size = message.length (oscPath);

    std::vector<std::byte> serialised;
    serialised.resize (size);
    message.serialise (oscPath, serialised.data(), nullptr);
    auto result = sendToAll (serialised);

    jassert (result); // Error sending message.
}

} // namespace mrlab::controller
