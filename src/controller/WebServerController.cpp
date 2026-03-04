/*
    WebServerController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "WebServerController.h"
#include "WebSocketController.h"
#include <util/Logger.h>
#include <Globals.h>

namespace mrlab::controller
{

WebServerController::WebServerController (OscController& oscController)
{
    mg_init_library (0);

    webSocketController = std::make_unique<WebSocketController> (oscController);
}

WebServerController::~WebServerController()
{
    stop();
    mg_exit_library();
}

bool WebServerController::start()
{
    if (civetServer != nullptr)
    {
        Logger::logWarn ("WebServerController: Ignoring start request, server appears to be running already.");
        return false;
    }

    const auto documentRootDir = Globals::getWebServerDocumentRootDir();
    const auto documentRoot = documentRootDir.getFullPathName();

    if (documentRootDir.isDirectory())
        Logger::logInfo (juce::String ("WebServerController: WebGUI document root is: ") + documentRoot);
    else
        Logger::logError (juce::String ("WebServerController: WebGUI document root does not exist: ") + documentRoot);

    std::vector<std::string> civetOptions;

    civetOptions.push_back ("document_root");
    civetOptions.push_back (documentRoot.toStdString());
    civetOptions.push_back ("listening_ports");
    civetOptions.push_back (std::to_string (Globals::getWebServerListeningPort()));
    civetOptions.push_back ("websocket_timeout_ms");
    civetOptions.push_back (std::to_string (5000));
    civetOptions.push_back ("enable_websocket_ping_pong");
    civetOptions.push_back ("yes");

    civetServer = std::make_unique<CivetServer> (civetOptions);

    if (civetServer->getContext() == nullptr)
    {
        Logger::logFatal ("WebServerController: Cannot initialize webserver, port " + juce::String (Globals::getWebServerListeningPort()) + " (TCP) occupied?");

        civetServer.reset();
        return false;
    }

    civetServer->addWebSocketHandler (Globals::getWebSocketUri(), *webSocketController);

    return true;
}

bool WebServerController::stop()
{
    if (civetServer == nullptr)
        return true;

    // Try to be gentle.
    webSocketController->closeAll();
    civetServer->close();
    civetServer->removeHandler (Globals::getWebSocketUri());

    civetServer.reset();

    if (webSocketController->getNumEndpoints() > 0)
    {
        Logger::logError ("WebServerController::stop(): Dangling WebSocket endpoints after webserver has been stopped.");
        return false;
    }

    return true;
}

} // namespace mrlab::controller
