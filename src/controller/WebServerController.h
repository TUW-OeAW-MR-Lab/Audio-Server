/*
    WebServerController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <memory>
#include <juce_core/juce_core.h>
#include <CivetServer.h>
#include "AppController.h"
#include "AppHandle.h"

namespace mrlab::controller
{
class MainController;

//==============================================================================
/** Controller that wraps a single webserver with websocket functionality.

    Both the http and the websocket facility listen to the same port
    specified in the Config object, so any connection to this can
    switch to the WebSocket protocol by means of an http protocol
    upgrade request.

    For the implementation, civetweb (https://github.com/civetweb/civetweb)
    is used.
 */
class WebServerController : public CivetWebSocketHandler,
                            public AppController::Listener,
                            public AppHandle::Listener
{
public:
    //==============================================================================
    WebServerController (MainController& mainControllerIn);
    ~WebServerController() override;

    /** Start the webserver.

        @return true on success, false on failure or server already running.
    */
    bool start();

    /** Stop the webserver.

        @return true on success, false if server was not running.
    */
    bool stop();

    /** Send a message to all connected clients.

        @param message Serialised binary message to send.

        @return true on success, false on error.
     */
    bool sendToAll (const std::vector<std::byte>& message);

    // CivetWebSocketHandler interface.
    bool handleConnection (CivetServer* server, const mg_connection* conn) override;
    void handleReadyState (CivetServer* server, mg_connection* conn) override;
    bool handleData (CivetServer* server, mg_connection* conn, int bits, char* data, size_t data_len) override;
    void handleClose (CivetServer* server, const mg_connection* conn) override;

    // AppController::Listener interface.
    void appAdded (AppHandle& app) override { app.addListener (this); }
    void appWillBeRemoved (AppHandle& app) override { app.removeListener (this); }

    // AppHandle::Listener interface.
    void appStateChanged (AppHandle& app, AppHandle::AppState newState) override;

private:
    //==============================================================================
    MainController& mainController;

    std::unique_ptr<CivetServer> civetServer; ///< Wrapped civetweb server instance.
    std::vector<mg_connection*> clients;  ///< Currently established client connections.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebServerController)
};

} // namespace mrlab::controller
