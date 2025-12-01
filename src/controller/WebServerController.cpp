/*
    WebServerController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "WebServerController.h"
#include "AppConfigController.h"
#include "AppController.h"
#include "AppHandle.h"
#include <iostream>

namespace mrlab::controller
{

WebServerController::WebServerController (AppController& inAppController)
    : appController (inAppController)
{
    mg_init_library (0);
}

WebServerController::~WebServerController()
{
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

bool WebServerController::handleConnection (CivetServer* server, const struct mg_connection* conn)
{
    juce::ignoreUnused (server, conn);

    std::cout << "WebSocket connection request" << std::endl;

    return true;
}

void WebServerController::handleReadyState (CivetServer* server, struct mg_connection* conn)
{
    juce::ignoreUnused (server, conn);

    std::cout << "WebSocket connection ready" << std::endl;
}

bool WebServerController::handleData (CivetServer* server, struct mg_connection* conn, int bits, char* data, size_t data_len)
{
    juce::ignoreUnused (server, conn, bits, data_len);

    // when receiving OSC, the address will be zero-padded in any case.
    std::cout << "WebSocket data received: " << data << std::endl;

    /* Dirty OSC path matching from here for initial testing.
       Eventually, this will be done by a proper OSC message dispatcher (IMRV-29).
     */
    const auto parts = juce::StringArray::fromTokens (data + 1, "/", ""); // strip initial '/'

    const auto appId = juce::Identifier (parts[0]);

    if (appController.getApps().contains (appId))
    {
        auto& handle = appController.getApp (appId);

        if (parts[1] == "launch")
            juce::MessageManager::callAsync ([&] { handle.start(); });
        else if (parts[1] == "quit")
            juce::MessageManager::callAsync ([&] { handle.stop(); });
    }

    return true;
}

void WebServerController::handleClose (CivetServer* server, const struct mg_connection* conn)
{
    juce::ignoreUnused (server, conn);

    std::cout << "WebSocket connection closing" << std::endl;
}

} // namespace mrlab::controller
