/*
    OscAgent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include <util/Logger.h>
#include "OscController.h"
#include "OscEndpoint.h"
#include <osc/Address.h>
#include <osc/Error.h>
#include <osc/Message.h>

namespace mrlab::controller
{

//==============================================================================
/** Base class for controllers acting upon received OpenSoundControl (OSC) messages.

    This class provided functions for subclasses to register OSC
    method handlers (callbacks) for certain OSC paths that are called
    upon received messages. It also provides functions for sending
    OSC messages (as replies or broadcast).

    @see OscController
    @see OscEndpoint
 */
class OscAgent
{
public:
    //==============================================================================
    virtual ~OscAgent() { oscController.removeAgent (*this); }

protected:
    //==============================================================================
    OscAgent (OscController& oscControllerIn)
        : oscController (oscControllerIn)
    {
        oscController.addAgent (*this);
    }

    /** Add and OSC message handler with a type signature.

        @param path OSC path to listen on, wildcards are supported.
        @param types Message signature for argument type coercion, use "" for no arguments.
        @param handler Handler function.

        @return true if successfully added, false on error.

        @note If the handler's first parameter is OscEndpoint*, it
              will be passed the source (sender) of the message or
              nullptr if not avaiable (e.g., for messages that were
              dispatched internally).

        @see OscController::addMethod
     */
    template <class Handler>
    bool addMethod (std::string_view path, std::string_view types, Handler&& handler) const
    {
        return oscController.addMethod (*this, path, types, std::function (std::forward<Handler> (handler)));
    }

    /** Add and OSC message handler without type signature (receives all arguments).

        @param path OSC path to listen on, wildcards are supported.
        @param handler Handler function.

        @return true if successfully added, false on error.

        @note If the handler's first parameter is OscEndpoint*, it
              will be passed the source (sender) of the message or
              nullptr if not avaiable.

        @see OscController::addMethod
     */
    template <class Handler>
    bool addMethod (std::string_view path, Handler&& handler) const
    {
        return oscController.addMethod (*this, path, nullptr, std::function (std::forward<Handler> (handler)));
    }

    /** Broadcast an OSC message to all registered endpoints.

        @param path OSC path to send.
        @param msg OSC message to send.
        @param origin Optional source endpoint of the message.

        @note In case of forwarding messages that were received by
              another endpoint, the origin parameter should reflect
              the original receiver in order to allow for filtering,
              e.g., to avoid echoes or loops.

        @see OscController::broadcast
     */
    void broadcast (std::string_view path, const lo::Message& msg, const OscEndpoint* origin = nullptr) const
    {
        oscController.broadcast (path, msg, origin);
    }

    /** Send an OSC message via a specific endpoint.

        This is mainly used for replies to state queries or error
        responses.

        @param destination Endpoint to use for sending.
        @param path OSC path to send.
        @param msg OSC message to send.
        @param origin Optional source endpoint of the message.

        @note In case of forwarding messages that were received by
              another endpoint, the origin parameter should reflect
              the original receiver in order to allow for filtering,
              e.g., to avoid echoes or loops.

        @see OscController::send
     */
    void send (OscEndpoint& destination, std::string_view path, const lo::Message& msg, const OscEndpoint* origin = nullptr) const
    {
        oscController.send (destination, path, msg, origin);
    }

    /** Send an OSC message as a reply or broadcast.

        If the destination parameter is nullptr, the message will be
        broadcasted to all endpoints, otherwise it will be sent via
        the specified endpoint. This might facilitate functions that
        both broadcast state updates as well as reply to individual
        state queries.

        @param destination Destination endpoint or nullptr for broadcast.
        @param path OSC path to send.
        @param msg OSC message to send.

        @note In case of forwarding messages that were received by
              another endpoint, the origin parameter should reflect
              the original receiver in order to allow for filtering,
              e.g., to avoid echoes or loops.
    */
    void sendOrBroadcast (OscEndpoint* destination, std::string_view path, const lo::Message& msg, const OscEndpoint* origin = nullptr) const
    {
        if (destination)
            send (*destination, path, msg, origin);
        else
            broadcast (path, msg, origin);
    }

    /** Send an OSC error message (/error) and report it.

        If destination is nullptr, the error will be only logged.

        @tparam logLevel Log level for log message.
        @param destination Endpoint for the error response or nullptr.
        @param error OSC error that occurred.
        @param args Additional arguments to include in the error message/report.
     */
    template <Logger::LogLevel logLevel = Logger::LogLevel::warn, class... Args>
    void sendError (OscEndpoint* destination, osc::Error error, Args... args)
    {
        const auto errorMsg = osc::ErrorDescription::get (error);
        std::stringstream logMsg;

        logMsg << "OscAgent: OSC /error to " << (destination ? destination->getPeer() : "<nn>") << ": ";
        logMsg << int32_t (error) << " (" << errorMsg;
        ((logMsg << ", " << args), ...) << ").";

        Logger::log (logLevel, logMsg.str());

        if (! destination)
            return;

        auto msg = osc::Message (int32_t (error), errorMsg, std::forward<Args> (args)...);

        send (*destination, osc::Address::error, msg);
    }

private:
    //==============================================================================
    OscController& oscController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscAgent)
};

} // namespace mrlab::controller
