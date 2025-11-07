/*
    OscController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "OscController.h"
#include "MainController.h"
#include <lo/lo_cpp.h>

namespace mrlab::controller
{

OscController::OscController (MainController& newMainController)
    : mainController (newMainController)
{
    addServer ("_main", juce::String(), 7081);
}

OscController::~OscController()
{
}

bool OscController::addServer (const juce::Identifier& id, juce::String subPath, int port)
{
    auto [iter, success] = servers.try_emplace (id, std::make_unique<lo::ServerThread> (port));

    if (! success)
    {
        jassertfalse; // Server with this key already exists!
        return false;
    }

    iter->second->add_method (nullptr, nullptr, [this] (std::string_view path, const lo::Message& message)
    {
        handleIncomingMessage (path, message);
    });

    // TODO: implement subPath handling
    juce::ignoreUnused (subPath);

    iter->second->start();

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

void OscController::handleIncomingMessage (std::string_view path, const lo::Message& message)
{
    std::cout << "OscController: received " << path << " with " << message.argc() << " args." << std::endl;

    listeners.call (&Listener::messageReceived, path, message);
}

} // namespace mrlab::controller
