/*
    OscController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "OscController.h"
#include "MainController.h"
#include <lo/lo_cpp.h>
#include <util/Logger.h>
#include <Globals.h>

namespace mrlab::controller
{

OscController::OscController (MainController& newMainController)
    : mainController (newMainController)
{
    addMainServer (Globals::getOscListeningPort());

    // Temporary hardcoded app-specific relay server.
    addSubPathServer (juce::Identifier ("pd_fly"), "/app/pd_fly/osc", 9336, "localhost", 10003);
    addSubPathServer (juce::Identifier ("reaper_reverb"), "/app/reaper_reverb/osc", 9338, "localhost", 10005);
    addSubPathServer (juce::Identifier ("pd_jungle"), "/app/pd_jungle/osc", 9340, "localhost", 10007);
    addSubPathServer (juce::Identifier ("reaper_music"), "/app/reaper_music/osc", 9342, "localhost", 10009);
}

OscController::~OscController()
{}

bool OscController::addSubPathServer (const juce::Identifier& id, std::string subPath, int listenPort, const juce::String& destination, int destinationPort)
{
    jassert (servers.contains (mainServerId)); // Main server should be initialised first!

    if (! addToServers (id, listenPort))
        return false;

    auto& subServer = servers.at (id);
    auto& mainServer = servers.at (mainServerId);

    jassert (subPath.starts_with ('/')); // OSC subpath needs a leading '/'!
    jassert (! subPath.ends_with ('/')); // OSC subpath must not have a trailing '/'!

    // Pattern-matching handler for transparent app-specific communication.
    mainServer->add_method (subPath + "/*", nullptr, [&subServer, subPathLength = subPath.length(), dest = destination.toStdString(), destinationPort] (std::string_view path, const lo::Message& message) {
        auto addr = lo::Address (dest, destinationPort);

        // Forward to subserver with subPath stripped from the path.
        auto strippedPath = path.substr (subPathLength);

        /* We would like to use the C++ interface here but it does not provide
           a working overload for the statement below, so we fall back to the C
           interface.

           addr.send_from (*subServer, strippedPath, message);
        */
        lo_send_message_from (addr, lo_server (*subServer), strippedPath.data(), message);

        return 1; // Continue searching for other handlers.
    });

    // Catch-all handler for return messages from app.
    subServer->add_method (nullptr, nullptr, [this, sub = std::move (subPath)] (std::string_view path, const lo::Message& message) {
        // Should be forwarded to main server with subPath added to app-local path.
        // For now, just send to WebSocket clients (webgui).
        auto fullPath = sub + std::string (path);
        auto size = message.length (fullPath);

        std::vector<std::byte> serialised;
        serialised.resize (size);
        message.serialise (fullPath, serialised.data(), nullptr);
        auto result = mainController.getWebServerController().sendToAll (serialised);
        jassertquiet (result);
    });

    subServer->start();

    return true;
}

bool OscController::removeServer (const juce::Identifier& id)
{
    if (! servers.contains (id))
    {
        jassertfalse; // Unknown key.
        return false;
    }

    // TODO implement
    return true;
}

bool OscController::dispatchRaw (std::span<std::byte> data)
{
    if (! servers.contains (mainServerId))
    {
        jassertfalse; // No main server present.
        return false;
    }

    return servers.at (mainServerId)->dispatch_data (data.data(), data.size()) >= 0;
}

bool OscController::addMainServer (int port)
{
    if (! addToServers (mainServerId, port))
        return false;

    auto& server = servers.at (mainServerId);

    // Pattern-matching handler for app-control messages.
    server->add_method ("/app/*/control", "s", [this] (std::string_view path, const lo::Message& message) {
        handleIncomingAppControlMessage (path, message);

        return 1; // Continue searching for other handlers.
    });

    // Catch-all handler for log.
    server->add_method (nullptr, nullptr, [this] (std::string_view path, const lo::Message& message) {
        handleIncomingMessage (path, message);

        return 1; // Continue searching for other handlers.
    });

    server->start();

    return true;
}

bool OscController::addToServers (const juce::Identifier& id, int port)
{
    auto [iter, success] = servers.try_emplace (id, std::make_unique<lo::ServerThread> (port));

    jassert (success); // A server with this key already exists!

    return success;
}

void OscController::handleIncomingMessage (std::string_view path, const lo::Message& message)
{
    Logger::logInfo (juce::String ("OscController [catch-all]: received ") + std::string (path) + " with " + juce::String (message.argc()) + " args.");

    listeners.call (&Listener::messageReceived, path, message);
}

void OscController::handleIncomingAppControlMessage (std::string_view path, const lo::Message& message)
{
    Logger::logInfo (juce::String ("OscController [app-ctrl]: received ") + std::string (path) + " with " + juce::String (message.argc()) + " args.");

    jassert (message.argc() == 1); // Unexpected number of message arguments!

    path.remove_prefix (1);                                                      // strip initial '/'
    const auto pathElems = juce::StringArray::fromTokens (path.data(), "/", ""); // strip initial '/'
    const auto appId = juce::Identifier (pathElems[1]);
    const auto command = std::string_view (&message.argv()[0]->s);

    auto& appController = mainController.getAppController();

    if (appController.getApps().contains (appId))
    {
        auto& handle = appController.getApp (appId);

        if (command == "launch")
            juce::MessageManager::callAsync ([&] { handle.start(); });
        else if (command == "quit")
            juce::MessageManager::callAsync ([&] { handle.stop(); });
    }
}

} // namespace mrlab::controller
