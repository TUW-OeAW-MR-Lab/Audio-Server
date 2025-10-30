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

namespace mrlab::controller
{
class AppController;

//==============================================================================
/** Controller that wraps a single webserver with websocket functionality.

    Both the http and the websocket facility listen to the same port
    specified in the Config object, so any connection to this can
    switch to the WebSocket protocol by means of an http protocol
    upgrade request.

    For the implementation, civetweb (https://github.com/civetweb/civetweb)
    is used.
 */
class WebServerController : public CivetWebSocketHandler
{
public:
    //==============================================================================
    /** Configuration class for the webserver. */
    struct Config
    {
        uint16_t listenPort;     ///< Port to listen to (should bind to all local interfaces).
        juce::File documentRoot; ///< Root directory of the file resources to serve via http.
    };

    //==============================================================================
    WebServerController (AppController& inAppController);
    ~WebServerController() override;

    /** Start the webserver.

        @param config Configuration to use.

        @return true on success, false on failure or server already running.
    */
    bool start (const Config& config);

    /** Stop the webserver.

        @return true on success, false if server was not running.
    */
    bool stop();

    // CivetWebSocketHandler interface.
    bool handleConnection (CivetServer* server, const struct mg_connection* conn) override;
    void handleReadyState (CivetServer* server, struct mg_connection* conn) override;
    bool handleData (CivetServer* server, struct mg_connection* conn, int bits, char* data, size_t data_len) override;
    void handleClose (CivetServer* server, const struct mg_connection* conn) override;

private:
    //==============================================================================
    AppController& appController;

    std::unique_ptr<CivetServer> civetServer; ///< The wrapped civetweb server instance.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebServerController)
};

} // namespace mrlab::controller
