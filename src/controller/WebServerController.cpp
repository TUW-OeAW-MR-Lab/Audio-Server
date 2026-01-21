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
#include <util/Logger.h>
#include <Config.h>

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

bool WebServerController::start()
{
    if (civetServer != nullptr)
    {
        Logger::logWarn ("WebServerController: Ignoring start request,server appears to be running already.");
        return false;
    }

    const auto documentRootDir = Config::getWebServerDocumentRootDir();
    const auto documentRoot = documentRootDir.getFullPathName();

    if (documentRootDir.isDirectory())
        Logger::logInfo (juce::String ("WebServerController: WebGUI document root is: ") + documentRoot);
    else
        Logger::logWarn (juce::String ("WebServerController: WebGUI document root does not exist: ") + documentRoot);

    std::vector<std::string> civetOptions;

    civetOptions.push_back ("document_root");
    civetOptions.push_back (documentRoot.toStdString());
    civetOptions.push_back ("listening_ports");
    civetOptions.push_back (std::to_string (Config::getWebServerListeningPort()));
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

    jassertquiet (server == civetServer.get());

    Logger::logInfo ("WebSocket connection request");

    return true;
}

void WebServerController::handleReadyState (CivetServer* server, mg_connection* conn)
{
    jassertquiet (server == civetServer.get());
    jassert (std::find (clients.begin(), clients.end(), conn) == clients.end()); // We already know this client?

    clients.push_back (conn);

    Logger::logInfo ("WebSocket connection ready");
}

bool WebServerController::handleData (CivetServer* server, mg_connection* conn, int bits, char* data, size_t data_len)
{
    juce::ignoreUnused (server, conn);

    jassert ((bits & 0xf0) == 0x80); // Client sends fragmented data or extension bits set, but we don't support that.

    const auto opcode = bits & 0x0f;

    if (opcode == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE)
    {
        Logger::logInfo (juce::String ("WebSocket connection close request received: ") + juce::String (data));
        return false;
    }

    jassert (opcode == MG_WEBSOCKET_OPCODE_BINARY); // We expect OSC messages to be sent in binary websocket frames.

    // when receiving OSC, the address will be zero-padded in any case.
    Logger::logInfo (juce::String ("WebSocket frame received: ") + juce::String (data));

    mainController.getOscController().dispatchRaw (std::span ((std::byte*) (data), data_len));

    return true;
}

void WebServerController::handleClose (CivetServer* server, const mg_connection* conn)
{
    jassertquiet (server == civetServer.get());

    auto num = std::erase (clients, conn);

    jassertquiet (num > 0); // We don't know this client!

    Logger::logInfo ("WebSocket connection closed");
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

    jassertquiet (result); // Error sending message.
}

} // namespace mrlab::controller
