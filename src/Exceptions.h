/*
    Exceptions.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <string_view>

//==============================================================================
namespace mrlab
{

//==============================================================================
/** Exception that is thrown when there is no config for the given id. */
class ConfigNotFoundException : public std::runtime_error
{
public:
    ConfigNotFoundException (const juce::Identifier& id)
        : std::runtime_error ("ConfigNotFoundException: No config found for id " + id.toString().toStdString())
    {}
};

/** Exception that is thrown when the validation of a YAML config failed. */
class ConfigInvalidException : public std::runtime_error
{
public:
    ConfigInvalidException (const std::string& what)
        : std::runtime_error ("ConfigInvalidException: " + what)
    {}
};

/** Exception that is thrown when a YAML config cannot be used in a certain context.

    This may indicate that a config section is missing but required.
*/
class ConfigUnusableException : public std::runtime_error
{
public:
    ConfigUnusableException (const std::string& what)
        : std::runtime_error ("ConfigUnusableException: " + what)
    {}
};

//==============================================================================
/** Exception that is thrown when there is no app for the given id. */
class AppNotFoundException : public std::runtime_error
{
public:
    AppNotFoundException (const juce::Identifier& id)
        : std::runtime_error ("AppNotFoundException: No app found with id " + id.toString().toStdString())
    {}
};

//==============================================================================
/** Server is invalid or could not be created (e.g., due to an occupied port). */
class ServerInvalidException : public std::runtime_error
{
public:
    ServerInvalidException (const std::string& what)
        : std::runtime_error ("ServerInvalidException: " + what)
    {}
};

/** Destination address is invalid or indicates an unsupported protocol. */
class DestinationInvalidException : public std::runtime_error
{
public:
    DestinationInvalidException (const std::string& what)
        : std::runtime_error ("DestinationInvalidException: " + what)
    {}
};

/** Exception indicating that sending OSC an endpoint failed. */
class SendingOscFailedException : public std::runtime_error
{
public:
    SendingOscFailedException (const std::string& what)
        : std::runtime_error ("SendingOscFailedException: " + what)
    {}
};

/** Indicating that an OSC message's path or path segment is malformed. */
class OscPathInvalidException : public std::runtime_error
{
public:
    OscPathInvalidException (const std::string& what)
        : std::runtime_error ("OscPathInvalidException: " + what)
    {}
};

//==============================================================================
/** Exception while processing the Prodigy JSON state tree. */
class ProdigyJsonException : public std::runtime_error
{
public:
    ProdigyJsonException (const std::string& what)
        : std::runtime_error ("ProdigyJsonException: " + what)
    {}
};

/** Discovered a corrupt state while processing the Prodigy JSON state tree. */
class ProdigyJsonCorruptStateException : public ProdigyJsonException
{
public:
    ProdigyJsonCorruptStateException (const std::string& what)
        : ProdigyJsonException ("ProdigyJsonCorruptStateException: " + what)
    {}
};

/** Exception concerning communication with the matrix via the Prodigy JSON control protocol. */
class ProdigyJsonProtocolException : public ProdigyJsonException
{
public:
    ProdigyJsonProtocolException (const std::string& what)
        : ProdigyJsonException ("ProdigyJsonProtocolException: " + what)
    {}
};

/** A referenced object could not be found in the Prodigy JSON state tree. */
class ProdigyJsonStateNotFoundException : public ProdigyJsonException
{
public:
    ProdigyJsonStateNotFoundException (const std::string& what)
        : ProdigyJsonException ("ProdigyJsonStateNotFoundException: " + what)
    {}
};

/** Expected an array index while processing the Prodigy JSON state tree. */
class ProdigyJsonNoArrayIndexException : public ProdigyJsonException
{
public:
    ProdigyJsonNoArrayIndexException (const std::string& what)
        : ProdigyJsonException ("ProdigyJsonNoArrayIndexException: " + what)
    {}
};

} // namespace mrlab
