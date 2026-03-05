/*
    OscEndpoint.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>
#include "OscController.h"

namespace lo
{
class Message;
}

namespace mrlab::controller
{

//==============================================================================
/** Base class for endpoints of an OpenSoundControl (OSC) communication.

    An OscEndpoint implements a certain transport method for OSC
    messages, such as datagram (UDP) or stream (TCP) based transport.
    This class provides an abstract interface for delivering (sending)
    OSC messages and for dispatching received messages to their
    respective handlers.

    @see OscController
 */
class OscEndpoint
{
public:
    //==============================================================================
    virtual ~OscEndpoint() { oscController.removeEndpoint (this); }

    /** Get a string representation of the remote side of the endpoint.

        Depending on the implementation, this may be an url or an ip
        address/port pair.

        @return A string representation of the peer.
     */
    const std::string& getPeer() const {
        std::lock_guard l (peerAccess);
        return peer;
    }

protected:
    //==============================================================================
    friend class OscController;

    OscEndpoint (OscController& oscControllerIn)
        : oscController (oscControllerIn)
    {
        oscController.addEndpoint (this);
    }

    /** Interface for sending, to be implemented by subclasses.

        @param path OSC path of the message.
        @param msg Message object encoding the message arguments.
        @param origin Source endpoint of the message or nullptr if not forwarding.

        @throws SendingOscFailedException.
     */
    virtual void send (std::string_view path, const lo::Message& msg, const OscEndpoint* origin) = 0;

    /** Parse and dispatch a raw binary block containing OSC data.

        The block will be moved in order to avoid copying.

        @param data Data block to dispatch.
     */
    void dispatch (std::vector<std::byte>&& data)
    {
        oscController.dispatch (std::move (data), this);
    }

    /** Parse and dispatch a raw binary block containing OSC data.

        The block will be copied from the provided view.

        @param data View to the data block to dispatch.
    */
    void dispatch (std::span<const std::byte> data) { oscController.dispatch (data, this); }

    /** Parse and dispatch an OSC message provided as an OSC path and message object.

        @param path OSC path of the message.
        @param msg Message object encoding the message arguments.
     */
    void dispatch (std::string_view path, const lo::Message& msg)
    {
        oscController.dispatch (path,msg, this);
    }

    /** Set the string representation of the remote side of the endpoint.

        Depending on the implementation, this may be an url or an ip
        address/port pair.

        @param newPeer String representation of the peer.
     */
    void setPeer (std::string_view newPeer) {
        std::lock_guard l (peerAccess);
        peer = newPeer;
    }

private:
    //==============================================================================
    OscController& oscController;
    std::string peer { "<nn>" };   ///< Remote side identification for logging.
    mutable std::mutex peerAccess; ///< Mutex for concurrent peer member access.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscEndpoint)
};

} // namespace mrlab::controller
