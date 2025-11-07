/*
    OscController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <vector>
#include <memory>
#include <juce_core/juce_core.h>
#include <util/ListenerInterface.h>

namespace lo
{
class ServerThread;
class Message;
}

namespace mrlab::controller
{
class MainController;

//==============================================================================
/**
 */
class OscController
{
public:
    //==============================================================================
    /** Listener interface to get informed about incoming OSC messages. */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Called when a new OSC message is received.

            @param oscPath OSC path (aka OSC address) the message is addressed to.
            @param oscMessage Message object containing the payload.
         */
        virtual void messageReceived (std::string_view path, const lo::Message& message) = 0;
    };

    //==============================================================================
    /**

     */
    OscController (MainController& newMainController);

    /**

     */
    ~OscController();

    /**

     */
    bool addServer (const juce::Identifier& id, juce::String subPath, int port);

    /**

     */
    bool removeServer (const juce::Identifier& id);

private:
    //==============================================================================
    /**

     */
    void handleIncomingMessage (std::string_view path, const lo::Message& message);

    //==============================================================================
    MainController& mainController;
    std::map<juce::Identifier, std::unique_ptr<lo::ServerThread>> servers; ///< Managed server instances.

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscController)
};

} // namespace mrlab::controller
