/*
    OscController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "OscController.h"
#include "UdpEndpoint.h"
#include <Globals.h>
#include <util/Logger.h>
#include <osc/Address.h>
#include <osc/Util.h>
#include <Exceptions.h>
#include <lo/lo_cpp.h>

namespace mrlab::controller
{

OscController::OscController()
{}

OscController::~OscController()
{
    stop();
}

bool OscController::start()
{
    const auto port = Globals::getOscListeningPort();

    // Main server nominally listening on TCP, but just for message dispatching.
    server = std::make_unique<lo::Server> (port, LO_TCP, [this] (int num, const char* msg, const char* where) { errorHandler (num, msg, where); });

    if (! server->is_valid())
    {
        Logger::logFatal ("OscController: Cannot initialize OSC message handling, port " + juce::String (Globals::getOscListeningPort()) + " (TCP) occupied?");
        return false;
    }

    // Catch-all handler for listeners and logging OSC traffic (debug).
    server->add_method (nullptr, nullptr, [this] (std::string_view path, const lo::Message& msg) {
#if JUCE_DEBUG
        const auto src = currentSource ? currentSource->getPeer() : " <nn>";

        std::cout << "[OSC recv " << src << "]: " << path << " ";
        msg.print();
#endif

        listeners.call (&Listener::messageReceived, path, msg);

        return 1; // Continue searching for other handlers.
    });

    // Global handler for /ping response.
    server->add_method (osc::Address::ping, nullptr, [this] (const lo::Message& msg) {
        if (currentSource)
            send (*currentSource, osc::Address::pong, msg);
    });

    // Main endpoint actually listening on UDP.
    try
    {
        mainEndpoint = std::make_unique<UdpEndpoint> (*this, port, UdpEndpoint::DestinationMode::lastReceived);
    }
    catch (const ServerInvalidException& e)
    {
        Logger::logError (juce::String ("OscController: ") + e.what());
        return false;
    }

    mainEndpoint->start();

    return true;
}

bool OscController::stop()
{
    for (const auto& [agent, methods] : agents)
        removeAgent (*agent);

    mainEndpoint.reset();
    server.reset();

    return true;
}

bool OscController::addAgent (const OscAgent& agent)
{
    const auto& [iter, result] = agents.try_emplace (&agent);

    return result;
}

bool OscController::removeAgent (const OscAgent& agent)
{
    if (! agents.contains (&agent))
        return false;

    // Remove all osc methods that were registered by agent.
    for (const auto& method : agents.at (&agent))
        server->del_method (method);

    return agents.erase (&agent) == 1;
}

bool OscController::addEndpoint (OscEndpoint* endpoint)
{
    const auto& [iter, result] = endpoints.insert (endpoint);

    return result;
}

bool OscController::removeEndpoint (OscEndpoint* endpoint)
{
    return endpoints.erase (endpoint) == 1;
}

void OscController::broadcast (std::string_view path, const lo::Message& msg, const OscEndpoint* origin)
{
    for (auto* endpoint : endpoints)
        send (*endpoint, path, msg, origin);
}

void OscController::send (OscEndpoint& destination, std::string_view path, const lo::Message& msg, const OscEndpoint* origin)
{
    try
    {
        destination.send (path, msg, origin);
    }
    catch (const SendingOscFailedException& e)
    {
        Logger::logError (e.what());
    }
}

void OscController::dispatch (std::vector<std::byte>&& data, OscEndpoint* source)
{
    // Dispatch all data synchronously on the message thread.
    /* Dispatching each message using callAsync() might be expensive
       (allocation-wise) with high congestion, we might be better off
       with a separate OSC data queue and, e.g., a timer on the
       message thread.
     */
    juce::MessageManager::callAsync ([this, d = std::move (data), source] () mutable {
        currentSource = source;
        const auto result = server->dispatch_data (d.data(), d.size());
        currentSource = nullptr;

        jassertquiet (result >= 0);
    });

}

void OscController::dispatch (std::span<const std::byte> data, OscEndpoint* source)
{
    dispatch (std::vector (data.begin(), data.end()), source);
}

void OscController::dispatch (std::string_view path, const lo::Message& msg, OscEndpoint* source)
{
    /* Unless we have an updated (or patched) version of liblo that
       allows for directly dispatching OSC messages, we have to
       serialise the message to be dispatched.
     */
    const auto size = msg.length (path);

    std::vector<std::byte> data;
    data.resize (size);
    msg.serialise (path, data.data(), nullptr);

    dispatch (std::move (data), source);
}

void OscController::errorHandler (int num, const char* msg, const char* where) const
{
    Logger::logError (osc::Util::formatServerError (*server, "OscController: ", num, msg, where));
}

} // namespace mrlab::controller
