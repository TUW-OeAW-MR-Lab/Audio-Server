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

OscController::OscController (MainController& mainControllerIn)
    : mainController (mainControllerIn)
{
    addMainServer (Globals::getOscListeningPort());
}

OscController::~OscController()
{
    removeServer (mainServerId);
}

bool OscController::addSubPathServer (const juce::Identifier& id, std::string subPath, int listenPort, const juce::String& destination, int destinationPort)
{
    // Main server should be initialised first!
    if (! servers.contains (mainServerId))
    {
        Logger::logError ("OscController::addSubPathServer: Main server is not initialised, cannot add subserver.");
        return false;
    }

    if (! subPath.starts_with ('/') || subPath.ends_with ('/'))
    {
        Logger::logError ("OscController::addSubPathServer: Malformed OSC subpath '" + subPath + "' (must have a leading and no trailing '/').");
        return false;
    }

    if (! addToServers (id, listenPort))
    {
        Logger::logError ("OscController::addSubPathServer: Error adding subserver for id: " + id.toString());
        return false;
    }

    auto& subServer = servers.at (id);
    auto& mainServer = servers.at (mainServerId);

    // Pattern-matching handler for transparent app-specific communication.
    auto subPathHandler = [&subServer, subPathLength = subPath.length(), dest = destination.toStdString(), destinationPort] (std::string_view path, const lo::Message& message) {
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
    };

    auto [iter, success] = mainServerMethods.try_emplace (id, mainServer->add_method (subPath + "/*", nullptr, std::move (subPathHandler)));

    if (! success)
    {
        Logger::logError (juce::String ("OscController::addSubPathServer: Error adding message handler for OSC subpath ") + subPath + ", id: " + id.toString());
        removeServer (id);
        return false;
    }

    // Catch-all handler for return messages from app.
    subServer->add_method (nullptr, nullptr, [this, subPath] (std::string_view path, const lo::Message& message) {
        // Should be forwarded to main server with subPath added to app-local path.
        // For now, just send to WebSocket clients (webgui).
        auto fullPath = subPath + std::string (path);
        auto size = message.length (fullPath);

        std::vector<std::byte> serialised;
        serialised.resize (size);
        message.serialise (fullPath, serialised.data(), nullptr);
        auto result = mainController.getWebServerController().sendToAll (serialised);
        jassertquiet (result);
    });

    subServer->start();

    Logger::logInfo (juce::String ("OscController::addSubPathServer: Added OSC subpath server for '") + subPath + "', listenPort: " + juce::String (listenPort) + ", destination: [" + destination + ", " + juce::String (destinationPort) + "].");

    return true;
}

bool OscController::removeServer (const juce::Identifier& id)
{
    if (! servers.contains (id))
    {
        jassertfalse; // Unknown key.
        return false;
    }

    if (mainServerMethods.contains (id))
    {
        // Remove app-specific subpath message handler.
        servers.at (mainServerId)->del_method (mainServerMethods.at (id));
        mainServerMethods.erase (id);
    }

    servers.at (id)->stop();
    servers.erase (id);

    if (id != mainServerId)
        Logger::logInfo ("OscController::removeServer: Removed OSC subpath server for id: " + id.toString());

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
