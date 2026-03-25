/*
    OscController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <vector>
#include <span>
#include <memory>
#include <juce_core/juce_core.h>
#include <util/ListenerInterface.h>
#include <lo/lo_cpp.h>

namespace mrlab::controller
{
class OscEndpoint;
class UdpEndpoint;
class OscAgent;

//==============================================================================
/** Central controller managing OSC communication.

    OSC message handling is implemented using the lo library
    (https://github.com/radarsat1/liblo).

    This controller holds a central lo::Server instance that manages
    the entire OSC address space and dispatches OSC messages to their
    respective handlers. This instance does not serve a network
    interface directly although it does bind to an (unused and
    unserved) TCP port, because network transport and message
    dispatching are not performed by separate objects in liblo.

    Actual message transport is performed by instances of OscEndpoint,
    which allows for serving multiple protocols and transport schemes
    that share a single OSC address space for dispatching.

    The actual handlers for received OSC messages are managed by
    instances of OscAgent for specific messages or parts of the OSC
    address space (OSC address wildcards).

    OSC dispatching is performed on the message thread. The dispatch()
    interface makes sure that data that is received concurrently
    (e.g., by endpoint worker threads) will be deferred accordingly.

    @see OscEndpoint
    @see OscAgent
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

            @param path OSC path (aka OSC address) the message is addressed to.
            @param msg Message object containing the payload.
         */
        virtual void messageReceived (std::string_view path, const lo::Message& msg) = 0;
    };

    //==============================================================================
    /** Create an OscController.

        @note OSC data can be only dispatched and OSC methods
              (handlers) can only be added after the OSC address space
              has been initialised with start().
     */
    OscController();
    ~OscController();

    /** Initialise the OSC address space and start listening on the main port.

        This will instantiate the internal OscEndpoint for listening
        on the main UDP port and add OSC message handlers for the
        listener interface, logging, and the global OSC interface.
     */
    bool start();

    /** Stop listening and reset the OSC address space.

        This will also remove all registered OscAgents.
     */
    bool stop();

    /** Add an OscAgent to this controller.

        @param agent OscAgent to add.
        @return true on success, false if already added.
     */
    bool addAgent (const OscAgent& agent);

    /** Remove an OscAgent from this controller.

        This will remove all OSC methods (message handlers) that were
        added by this agent.

        @param agent OscAgent to remove.
        @return true on success, false if agent is unknown.
     */
    bool removeAgent (const OscAgent& agent);

    /** Add an OSC method (message handler).

        Multiple function signatures for the handler are supported by liblo:

        @li @code (std::string_view path, std::string_view types, lo_arg** argv, int argc, const lo::Message& message) @endcode
        @li @code (std::string_view path, std::string_view types, lo_arg** argv, int argc) @endcode
        @li @code (std::string_view path, const lo::Message& message) @endcode
        @li @code (std::string_view types, lo_arg** argv, int argc) @endcode
        @li @code (std::string_view types, lo_arg** argv, int argc, const lo::Message& message) @endcode
        @li @code (lo_arg** argv, int argc) @endcode
        @li @code (lo_arg** argv, int argc, const lo::Message& message) @endcode
        @li @code (const lo::Message& message) @endcode
        @li @code () @endcode

        Handlers may return an int indicating whether the OSC message
        has been successfully consumed (0) or whether further matching
        OSC methods should be considered (1).

        @param agent OscAgent registering the handler.
        @param path OSC path to listen on, wildcards are supported.
        @param types Message signature for argument type coercion. Use
               an empty string for no arguments or nullptr for no type
               signature, i.e., respond to all type signatures.
        @param handler Handler function.

        @return true if successfully added, false on error.

        @see lo::Server::add_method.

        @note The recommended way of adding OSC methods is to use the
              OscAgent::addOscMethod() delegate functions.

        @note While OSC methods are added (and removed) per agent, the
              method dispatcher provided by liblo is still global,
              i.e., methods added by multiple agents that are
              listening to the same OSC paths (or wildcards) may still
              prevent each other from being called, depending on
              whether their handler functions' return value requests
              to continue searching for other matching methods.
     */
    template <class Types, class Handler>
    bool addMethod (const OscAgent& agent, std::string_view path, Types types, Handler&& handler)
    {
        if (! server->is_valid())
            return false;

        if (! agents.contains (&agent))
            return false;

        agents.at (&agent).emplace_back (server->add_method (path, types, std::forward<Handler> (handler)));

        return true;
    }

    /** Add an OSC method (message handler) receiving the source OscEndpoint.

        The handler's first parameter of type OscEndpoint* will be
        assigned the source of the message or nullptr if not available
        (e.g., for messages that were dispatched internally).

        @param agent OscAgent registering the handler.
        @param path OSC path to listen on, wildcards are supported.
        @param types Message signature for argument type coercion. Use
               an empty string for no arguments or nullptr for no type
               signature, i.e., respond to all type signatures.
        @param handler Handler function.

        @return true if successfully added, false on error.

        @note The current implementation requires this handler to be
              explicitly wrapped in a std::function object. This
              overload accepts handler functions that do not return
              any value.

        @note The recommended way of adding OSC methods is to use the
              OscAgent::addOscMethod() delegate functions.
     */
    template <class Types, class... Args>
    bool addMethod (const OscAgent& agent, std::string_view path, Types types, std::function<void (OscEndpoint*, Args...)>&& handler)
    {
        return addMethod (agent, path, types, [this, h = std::move (handler)] (Args... args) { h (currentSource, args...); });
    }

    /** Add an OSC method (message handler) receiving the source OscEndpoint.

        The handler's first parameter of type OscEndpoint* will be
        assigned the source of the message or nullptr if not available
        (e.g., for messages that were dispatched internally).

        @param agent OscAgent registering the handler.
        @param path OSC path to listen on, wildcards are supported.
        @param types Message signature for argument type coercion. Use
               an empty string for no arguments or nullptr for no type
               signature, i.e., respond to all type signatures.
        @param handler Handler function.

        @return true if successfully added, false on error.

        @note The current implementation requires this handler to be
              explicitly wrapped in a std::function object. This
              overload accepts handler functions returning an int for
              indicating whether the message has been successfully
              consumed.

        @note The recommended way of adding OSC methods is to use the
              OscAgent::addOscMethod() delegate functions.
     */
    template <class Types, class... Args>
    bool addMethod (const OscAgent& agent, std::string_view path, Types types, std::function<int (OscEndpoint*, Args...)>&& handler)
    {
        return addMethod (agent, path, types, [this, h = std::move (handler)] (Args... args) { h (currentSource, args...); });
    }

    /** Add an OscEndpoint.

        Broadcast messages will be send via all registered endpoints.

        @param endpoint OscEndpoint to add.
        @return true on success, false if already added.
     */
    bool addEndpoint (OscEndpoint* endpoint);

    /** Remove an OscEndpoint.

        @param endpoint OscEndpoint to remove.
        @return true on success, false if endpoint is unknown.
    */
    bool removeEndpoint (OscEndpoint* endpoint);

    /** Broadcast an OSC message to all registered endpoints.

        Depending on their implementation and configuration, endpoints
        may filter or alter messages before actually sending them,
        e.g., for echo suppression, path rewriting or handling
        subscriptions to only a subset of OSC paths.

        @param path OSC path to send.
        @param msg OSC message to send.
        @param origin Optional source endpoint of the message.

        @note In case of forwarding messages that were received by
              another endpoint, the origin parameter should reflect
              the original receiver in order to allow for filtering,
              e.g., to avoid echoes or loops.

        @note In the context of an OscAgent, its delegate osc sending
              interface might be more convenient to use.
     */
    void broadcast (std::string_view path, const lo::Message& msg, const OscEndpoint* origin = nullptr);

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

        @note In the context of an OscAgent, its delegate osc sending
              interface might be more convenient to use.
     */
    void send (OscEndpoint& destination, std::string_view path, const lo::Message& msg, const OscEndpoint* origin = nullptr);

    /** Parse and dispatch a raw binary block containing OSC data.

        The block will be moved in order to avoid copying.

        @param data Data block to dispatch.
        @param source Endpoint which received the data or nullptr if none.

        @note Calling this is thread-safe, message dispatching will be
              delegated to the message thread internally.

        @note For dispatching from an endpoint implementation, the
              OscEndpoint delegate interface should be preferred.
     */
    void dispatch (std::vector<std::byte>&& data, OscEndpoint* source);

    /** Parse and dispatch a raw binary block containing OSC data.

        The block will be copied from the provided view.

        @param data View to the data block to dispatch.
        @param source Endpoint which received the data or nullptr if none.

        @note Calling this is thread-safe, message dispatching will be
              delegated to the message thread internally.

        @note For dispatching from an endpoint implementation, the
              OscEndpoint delegate interface should be preferred.
     */
    void dispatch (std::span<const std::byte> data, OscEndpoint* source);

    /** Dispatch an OSC message for a given OSC path (aka OSC address).

        @param data View to the data block to dispatch.
        @param source Endpoint which received the message or nullptr if none.

        @note Calling this is thread-safe, message dispatching will be
              delegated to the message thread internally.

        @note For dispatching from an endpoint implementation, the
              OscEndpoint delegate interface should be preferred.
     */
    void dispatch (std::string_view path, const lo::Message& msg, OscEndpoint* source);

private:
    //==============================================================================
    /** Error handler for lo::Server for logging purposes. */
    void errorHandler (int num, const char* msg, const char* where) const;

    /**
        @param path
        @param message
     */
    void handleIncomingAppControlMessage (std::string_view path, const lo::Message& message);

    //==============================================================================
    std::unique_ptr<lo::Server> server; ///< Main server instance keeping the OSC address space.

    std::unordered_set<OscEndpoint*> endpoints; ///< Endpoints for sending/receiving OSC data.
    std::unique_ptr<UdpEndpoint> mainEndpoint;  ///< Endpoint for main UDP listening/sending.

    std::map<const OscAgent*, std::vector<lo::Method>> agents; ///< Registered OscAgents.

    /** Source endpoint of the currently dispatched OSC data. */
    OscEndpoint* currentSource { nullptr };

    MRLAB_IMPLEMENT_LISTENER_INTERFACE

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscController)
};

} // namespace mrlab::controller
