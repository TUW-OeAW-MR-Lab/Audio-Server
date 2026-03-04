/*
    WebServerController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>

class CivetServer;

namespace mrlab::controller
{
class OscController;
class WebSocketController;

//==============================================================================
/** Controller that wraps a single webserver with websocket functionality.

    Both the http and the websocket facility listen to the same port
    specified in the Config object, so any connection to this can
    switch to the WebSocket protocol by means of an http protocol
    upgrade request.

    The websocket facility is managed by a sub-controller
    (WebSocketController).

    For the implementation, civetweb (https://github.com/civetweb/civetweb)
    is used.
 */
class WebServerController
{
public:
    //==============================================================================
    WebServerController (OscController& oscController);
    ~WebServerController();

    /** Start the webserver.

        @return true on success, false on failure or server already running.
    */
    bool start();

    /** Stop the webserver.

        @return true on success, false if there was an error to
                gracefully stop.
    */
    bool stop();

private:
    //==============================================================================
    std::unique_ptr<WebSocketController> webSocketController; ///< WebSocket endpoints controller.
    std::unique_ptr<CivetServer> civetServer;                 ///< Wrapped civetweb server instance.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebServerController)
};

} // namespace mrlab::controller
