/*
    MatrixOscAgent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

// For std::numeric_limits<>::min/max on Windows.
#ifndef NOMINMAX
#define NOMINMAX 1
#endif

#include "MatrixOscAgent.h"
#include <util/Logger.h>
#include <osc/Util.h>
#include <lo/lo_cpp.h>
#include <Exceptions.h>
#include <charconv>

namespace mrlab::controller
{

MatrixOscAgent::MatrixOscAgent (OscController& oscControllerIn, MatrixController& matrixControllerIn)
    : OscAgent (oscControllerIn),
      oscController (oscControllerIn),
      matrixController (matrixControllerIn)
{
    matrixController.addListener (this);
}

MatrixOscAgent::~MatrixOscAgent()
{
    matrixController.removeListener (this);
}

void MatrixOscAgent::stateChanged (MatrixController&, MatrixController::State state)
{
    // Query full JSON state tree from matrix after connecting.
    if (state == MatrixController::State::connected)
    {
        addCommandOscMethods();
        startTimer (prodigyPingIntervalMs);
        matrixController.sendMatrixMessage (makeMessage (Type::get));
    }
}

void MatrixOscAgent::handleMatrixMessage (nlohmann::json msg)
{
#if JUCE_DEBUG
    std::cout << "[JSON Prodigy recv]" << std::endl;
    std::cout << std::setw (2) << msg << std::endl;
#endif

    try
    {
        const auto& type = msg.at (Field::type).get_ref<const std::string&>();

        if (type == Type::get_resp)
            return handleGetRespMessage (std::move (msg));
        if (type == Type::update)
            return handleUpdateMessage (std::move (msg));
        if (type == Type::error)
            return handleErrorMessage (std::move (msg));
        if (type == Type::ack)
            return handleAckMessage (std::move (msg));

        Logger::logError ("MatrixOscAgent: Received unknown Prodigy JSON message with type: " + type);
    }
    catch (const std::runtime_error& e)
    {
        return Logger::logError (juce::String ("MatrixOscAgent: Failure to deal with incoming Prodigy JSON message: ") + e.what());
    }
}

void MatrixOscAgent::timerCallback()
{
    if (auto numPending = pendingMsg.size(); numPending > 0)
        Logger::logWarn (juce::String ("MatrixOscAgent: ") + juce::String (numPending) + " Prodigy JSON command message responses pending.");

    matrixController.sendMatrixMessage (makeMessage (Type::cmd, {}, Command::ping));
}

void MatrixOscAgent::handleGetRespMessage (nlohmann::json&& msg)
{
    jassert (msg.at (Field::type).get_ref<const std::string&>() == Type::get_resp);

    if (! removeMessageFromPending (msg))
        Logger::logDebug ("MatrixOscAgent: Received non-pending response for message seq: " + (msg.contains (Field::seq) ? nlohmann::to_string (msg.at (Field::seq)) : "<?>"));

    if (msg.contains (Field::obj) && ! msg.at (Field::obj).empty())
        return Logger::logError ("MatrixOscAgent: Received unsupported get_resp Prodigy JSON message with obj pointer: " + msg.at (Field::obj).dump());

    if (! matrix.is_null())
        return Logger::logError ("MatrixOscAgent: Received full state get_resp Prodigy JSON message but state has been already stored.");

    // Store full Prodigy state.
    matrix = msg.at (Field::payload);

    // Register catch-all OSC method handler for state getting.
    addStateGetterOscMethod (nlohmann::json::json_pointer());

    // Register OSC method handler for /settings subtree for state setting.
    addStateSetterOscMethod (nlohmann::json::json_pointer ("/settings"));
}

void MatrixOscAgent::handleUpdateMessage (nlohmann::json&& msg)
{
    jassert (msg.at (Field::type).get_ref<const std::string&>() == Type::update);

    auto update = [this] (const nlohmann::json& payload, nlohmann::json& state, std::string_view oscPath) {
        // Update local state from payload.
        state = payload;

        // Broadcast update to OSC clients.
        broadcastStateChange (oscPath, state);
    };

    try
    {
        forEachPrimitiveInPayload (std::move (update), msg.at (Field::payload));
    }
    catch (const ProdigyJsonException& e)
    {
        Logger::logError (juce::String ("MatrixOscAgent: Error processing Prodigy JSON 'update' message (") + e.what() + ").");
    }
}

void MatrixOscAgent::handleErrorMessage (nlohmann::json&& msg)
{
    jassert (msg.at (Field::type).get_ref<const std::string&>() == Type::error);

    Logger::logError (juce::String ("MatrixOscAgent: Received Prodigy JSON error for message seq: ") +
                      (msg.contains (Field::seq) ? nlohmann::to_string (msg.at (Field::seq)) : "<?>") + " (" + msg.at (Field::obj).get_ref<std::string&>() + ").");

    if (! removeMessageFromPending (msg))
        Logger::logDebug ("MatrixOscAgent: Received non-pending response for message seq: " + (msg.contains (Field::seq) ? nlohmann::to_string (msg.at (Field::seq)) : "<?>"));
}

void MatrixOscAgent::handleAckMessage (nlohmann::json&& msg)
{
    jassert (msg.at (Field::type).get_ref<const std::string&>() == Type::ack);

    Logger::logDebug (juce::String ("MatrixOscAgent: Received Prodigy JSON ack for message seq: ") +
                      (msg.contains (Field::seq) ? nlohmann::to_string (msg.at (Field::seq)) : "<?>"));

    if (! removeMessageFromPending (msg))
        Logger::logDebug ("MatrixOscAgent: Received non-pending response for message seq: " + (msg.contains (Field::seq) ? nlohmann::to_string (msg.at (Field::seq)) : "<?>"));
}

nlohmann::json MatrixOscAgent::makeMessage (std::string_view type, const nlohmann::json& payload, const nlohmann::json& obj)
{
    nlohmann::json msg;

    pendingMsg.insert (sequence);
    msg[Field::seq] = sequence++;

    msg[Field::type] = type;

    if (! payload.is_null())
        msg[Field::payload] = payload;

    if (! obj.is_null())
        msg[Field::obj] = obj;

    return msg;
}

bool MatrixOscAgent::broadcastStateChange (std::string_view path, const nlohmann::json& state, const OscEndpoint* origin) const
{
    osc::Message msg;

    if (! addJson2osc (msg, state))
    {
        Logger::logError ("MatrixOscAgent::broadcastStateChange: Error converting state value to OSC for path " + std::string (path));
        return false;
    }

    broadcast (path, msg, origin);

    return true;
}

bool MatrixOscAgent::removeMessageFromPending (const nlohmann::json& msg)
{
    if (! msg.contains (Field::seq))
        return false;

    if (const auto& seq = msg.at (Field::seq); seq.is_number_unsigned())
        return bool (pendingMsg.erase (seq.get<uint32_t>()));

    return false;
}

bool MatrixOscAgent::addJson2osc (osc::Message& osc, const nlohmann::json& json)
{
    switch (json.type())
    {
        case nlohmann::json::value_t::number_integer:
        case nlohmann::json::value_t::number_unsigned:
            osc.add (json.get<int32_t>());
            return true;

        case nlohmann::json::value_t::number_float:
            osc.add (json.get<float>());
            return true;

        case nlohmann::json::value_t::string:
            osc.add (json.get<std::string_view>());
            return true;

        case nlohmann::json::value_t::boolean:
            osc.add (int32_t (json.get<bool>()));
            return true;

        case nlohmann::json::value_t::null:
        case nlohmann::json::value_t::object:
        case nlohmann::json::value_t::array:
        case nlohmann::json::value_t::binary:
        case nlohmann::json::value_t::discarded:
        default:
            return false;
    }
}

bool MatrixOscAgent::osc2json (nlohmann::json& json, char type, lo_arg* osc)
{
    // Helper to assign to JSON value from a numerical type.
    auto assignNumber = [&json]<typename T> (T val) -> bool {
        switch (json.type())
        {
            case nlohmann::json::value_t::number_integer:
                json = int32_t (val);
                return true;

            case nlohmann::json::value_t::number_unsigned:
                if (val < T (0))
                    return false;

                json = uint32_t (val);
                return true;

            case nlohmann::json::value_t::number_float:
                json = float (val);
                return true;

            case nlohmann::json::value_t::boolean:
                json = bool (val);
                return true;

            case nlohmann::json::value_t::null:
            case nlohmann::json::value_t::string:
            case nlohmann::json::value_t::object:
            case nlohmann::json::value_t::array:
            case nlohmann::json::value_t::binary:
            case nlohmann::json::value_t::discarded:
            default:
                return false;
        }
    };

    switch (type)
    {
        case 'i':
            return assignNumber (osc->i);

        case 'f':
            return assignNumber (osc->f);

        case 's':
            if (json.is_string())
            {
                json = &osc->s;
                return true;
            }
            if (json.is_boolean())
            {
                std::string_view val (&osc->s);
                json = val == "true";
                return true;
            }
            return false;

        case 'T':
            if (json.is_boolean())
            {
                json = true;
                return true;
            }
            return false;

        case 'F':
            if (json.is_boolean())
            {
                json = false;
                return true;
            }
            return false;

        default:
            return false;
    }
}

nlohmann::json MatrixOscAgent::oscPath2objPointer (std::string_view oscPath)
{
    using namespace std::string_view_literals;

    // Strip "/matrix/".
    jassert (oscPath.starts_with (osc::Address::matrix));
    oscPath.remove_prefix (osc::Address::matrix.size() + 1); // +1 for trailing /

    // Split OSC path and build 'obj' JSON array.
    nlohmann::json obj;

    for (const auto& seg : std::views::split (oscPath, "/"sv))
    {
        const auto segment = std::string_view (seg.data(), seg.size());

        // Try to parse the segment as a number for array indexing, otherwise use as object key.
        try
        {
            obj.emplace_back (string2index (segment));
        }
        catch (const std::runtime_error&)
        {
            obj.emplace_back (segment);
        }
    }

    return obj;
}

std::string MatrixOscAgent::matrixOscPathFromJsonPointer (const nlohmann::json::json_pointer& pointer)
{
    auto oscPath = pointer.to_string();
    oscPath.insert (0, osc::Address::matrix);
    return oscPath;
}

uint32_t MatrixOscAgent::string2index (std::string_view string)
{
    uint32_t index;
    auto* last = string.data() + string.size();
    auto result = std::from_chars (string.data(), last, index);

    if (result.ptr == last)
        return index;

    throw ProdigyJsonNoArrayIndexException ("MatrixOscAgent: Cannot parse string to numerical index value.");
}

nlohmann::json::iterator MatrixOscAgent::findSubArrayForIndex (nlohmann::json& array, uint32_t index)
{
    jassert (array.is_array());

    // Search for inner array with matching index in state.
    auto subArrayMatches = [index] (const auto& sub) {
        if (! sub.is_array() || sub.empty() || ! sub[0].is_number_unsigned())
            throw ProdigyJsonCorruptStateException ("MatrixOscAgent: State array does not contain indexed sub-arrays.");

        return sub[0].template get<uint32_t>() == index;
    };

    return std::find_if (array.begin(), array.end(), std::move (subArrayMatches));
}

void MatrixOscAgent::addCommandOscMethods()
{
    addMethod (std::string (osc::Address::matrixcmd) += "/*", [this] (OscEndpoint* source, std::string_view path, std::string_view types, lo_arg** argv, int argc) {
        if (argc > 1)
            return sendError (source, osc::Error::matrixCommandUnsupportedNumArguments, path, argc);

        // Get wildcard path segment, i.e., the CMD selector (JSON obj).
        const auto cmd = osc::Util::getPathSegment (path, 1);

        // Assign payload from OSC argument.
        nlohmann::json payload;

        if (argc > 0)
        {
            switch (types.front())
            {
                case 'i':
                    payload = argv[0]->i;
                    break;

                case 'f':
                    payload = argv[0]->f;
                    break;

                case 's':
                    payload = &argv[0]->s;
                    break;

                case 'T':
                    payload = true;
                    break;

                case 'F':
                    payload = false;
                    break;

                /* We could support transparent base64 encoding of OSC
                   binary blobs to JSON strings here but let's stay reasonable.
                 */

                default:
                    return sendError (source, osc::Error::matrixCommandIncompatibleArgumentType, path, types.front());
            }
        }

        // Send CMD message.
        auto cmdMsg = makeMessage (Type::cmd, payload, cmd);
#if JUCE_DEBUG
        std::cout << "[JSON Prodigy send]" << std::endl;
        std::cout << std::setw (2) << cmdMsg << std::endl;
#endif
        matrixController.sendMatrixMessage (std::move (cmdMsg));
    });
}

void MatrixOscAgent::addStateGetterOscMethod (nlohmann::json::json_pointer top)
{
    auto oscPattern = matrixOscPathFromJsonPointer (top);
    oscPattern += "/*";

    addMethod (oscPattern, {}, [this, ptrTop = std::move (top)] (OscEndpoint* source, std::string_view path, const lo::Message&) {
        if (! source)
            return Logger::logWarn (std::string ("MatrixOscAgent: Discarding state query message without sender for OSC path: ") += path);

        auto sendStateReply = [this, source] (nlohmann::json& state, std::string_view oscPath) {
            osc::Message msg;

            if (! addJson2osc (msg, state))
                return Logger::logError ("MatrixOscAgent: Error converting state value to OSC for path " + std::string (oscPath) + ").");

            send (*source, oscPath, msg);
        };

        try
        {
            forEachPrimitiveMatchingOscPattern (std::move (sendStateReply), path, ptrTop);
        }
        catch (const ProdigyJsonStateNotFoundException& e)
        {
            sendError (source, osc::Error::matrixStateNotFound, path, e.what());
        }
        catch (const ProdigyJsonNoArrayIndexException& e)
        {
            sendError (source, osc::Error::matrixStateArrayIndexExpected, path, e.what());
        }
        catch (const ProdigyJsonException& e)
        {
            sendError<Logger::LogLevel::error> (source, osc::Error::matrixGeneric, path, e.what());
        }
    });
}

void MatrixOscAgent::addStateSetterOscMethod (nlohmann::json::json_pointer top)
{
    auto oscPattern = matrixOscPathFromJsonPointer (top);
    oscPattern += "/*";

    addMethod (oscPattern, [this, ptrTop = std::move (top)] (OscEndpoint* source, std::string_view path, std::string_view types, lo_arg** argv, int argc) {
        // OSC message with no arguments should be handled by state getter method.
        if (types.empty()) // argc == 0
            return 1;

        if (argc > 1)
        {
            sendError (source, osc::Error::matrixStateUnsupportedNumArguments, path, argc);
            return 0;
        }

        auto setStateAndBroadcast = [&, this] (nlohmann::json& state, std::string_view oscPath) {
            // Assign OSC argument to state value.
            if (! osc2json (state, types.front(), argv[0]))
            {
                sendError (source, osc::Error::matrixStateIncompatibleArgumentType, path, types.front());
                return;
            }

            // Apply state change to matrix.
            auto setMsg = makeMessage (Type::set, state, oscPath2objPointer (oscPath));
#if JUCE_DEBUG
            std::cout << "[JSON Prodigy send]" << std::endl;
            std::cout << std::setw (2) << setMsg << std::endl;
#endif
            matrixController.sendMatrixMessage (std::move (setMsg));

            // Broadcast state change to OSC clients.
            broadcastStateChange (oscPath, state, source);
        };

        try
        {
            forEachPrimitiveMatchingOscPattern (std::move (setStateAndBroadcast), path, ptrTop);
        }
        catch (const ProdigyJsonStateNotFoundException& e)
        {
            sendError (source, osc::Error::matrixStateNotFound, path, e.what());
        }
        catch (const ProdigyJsonNoArrayIndexException& e)
        {
            sendError (source, osc::Error::matrixStateArrayIndexExpected, path, e.what());
        }
        catch (const ProdigyJsonException& e)
        {
            sendError<Logger::LogLevel::error> (source, osc::Error::matrixGeneric, path, e.what());
        }

        return 0;
    });
}

template <std::invocable<const nlohmann::json&, nlohmann::json&, std::string_view> Func>
void MatrixOscAgent::forEachPrimitiveInPayload (Func&& func, const nlohmann::json& payload, nlohmann::json::json_pointer top)
{
    /* Helper for recursive state traversal.

       @param curPayload Current reference into input payload.
       @param curState Current reference into state, corresponding to p.
       @param curOscPath Recursively built OSC path matching the current payload/state references.
       @param r Recurse function (passed on).
     */
    auto recurse = [f = std::forward<Func> (func)] (const nlohmann::json& curPayload, nlohmann::json& curState, std::string_view curOscPath, auto&& r) -> void {

        if (curPayload.is_primitive() && curState.is_primitive())
        {
            f (curPayload, curState, curOscPath);
            return;
        }

        if (curPayload.is_object() && curState.is_object())
        {
            for (const auto& [key, value] : curPayload.items())
            {
                if (! curState.contains (key))
                    throw ProdigyJsonProtocolException (std::string ("MatrixOscAgent::forEachPrimitiveInPayload: Payload references unknown key in matrix state at ") += curOscPath);

                r (value, curState.at (key), osc::Util::appendPathSegment (curOscPath, key), r);
            }
            return;
        }

        if (curPayload.is_array() && curState.is_array())
        {
            for (const auto& elem : curPayload)
            {
                /* According to the Prodigy array semantics (Prodigy
                   JSON specification 2.2.8.1), we expect inner arrays
                   with [index, value] here.
                 */
                if (! elem.is_array() || elem.empty() || ! elem[0].is_number_unsigned())
                    throw ProdigyJsonProtocolException (std::string ("MatrixOscAgent::forEachPrimitiveInPayload: Payload array does not contain indexed sub-arrays at ") += curOscPath);

                const auto index = elem[0].get<uint32_t>();

                if (const auto sub = findSubArrayForIndex (curState, index); sub != curState.end())
                {
                    r (elem[1], (*sub)[1], osc::Util::appendPathSegment (curOscPath, std::to_string (index)), r);
                    continue;
                }

                throw ProdigyJsonCorruptStateException (std::string ("MatrixOscAgent::forEachPrimitiveInPayload: No matching state array for payload array index ") + std::to_string (index) + " at " += curOscPath);
            }
            return;
        }

        throw ProdigyJsonCorruptStateException (std::string ("MatrixOscAgent::forEachPrimitiveInPayload: Unsupported JSON value type in payload at ") += curOscPath);
    };

    if (! matrix.contains (top))
        throw ProdigyJsonCorruptStateException (std::string ("MatrixOscAgent::forEachPrimitiveInPayload: Cannot access sub-state pointed to by top at ") += top.to_string());

    recurse (payload, matrix.at (top), matrixOscPathFromJsonPointer (top), recurse);
}

template <std::invocable<nlohmann::json&, std::string_view> Func>
void MatrixOscAgent::forEachPrimitiveMatchingOscPattern (Func&& func, std::string_view oscPattern, nlohmann::json::json_pointer top)
{
    using namespace std::string_view_literals;

    // Prepare initial OSC path.
    auto initialOscPath = matrixOscPathFromJsonPointer (top);

    // Strip "/matrix/<top>" from oscPath.
    jassert (oscPattern.starts_with (initialOscPath));
    oscPattern.remove_prefix (initialOscPath.size() + 1); // +1 for trailing /

    auto oscSplit = std::views::split (oscPattern, "/"sv);

    /* Helper for recursive state traversal.

       @param curSegmentIt Iterator referencing the OSC path segment to process.
       @param curState Current reference into state.
       @param curOscPath Recursively built OSC path matching the current state reference.
       @param r Recurse function (passed on).
     */
    auto recurse = [f = std::forward<Func> (func), oscEndIt = oscSplit.end()] (auto curSegmentIt, nlohmann::json& curState, std::string_view curOscPath, auto&& r) -> void {

        if (curSegmentIt == oscEndIt)
        {
            // When reaching the end of the OSC path, we must have arrived at a state primitive.
            if (curState.is_primitive())
            {
                f (curState, curOscPath);
                return;
            }

            throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No primitive state value for OSC path at ") += curOscPath);
        }

        // Need a std::string here to obtain a c_str() for liblo matching functions.
        const auto curSegment = std::string ((*curSegmentIt).data(), (*curSegmentIt).size());
        ++curSegmentIt;

        if (curSegment.empty())
            throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: Empty OSC path segment following ") += curOscPath);

        // JSON primitive value while not at end of OSC path.
        if (curState.is_primitive())
            throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No further state hierarchy for '") + curSegment + "' below OSC path " += curOscPath);

        // JSON object: match OSC segment against member keys.
        if (curState.is_object())
        {
            // No wildcard pattern: direct match?
            if (! lo_string_contains_pattern (curSegment.c_str()))
            {
                if (curState.contains (curSegment))
                {
                    r (curSegmentIt, curState.at (curSegment), osc::Util::appendPathSegment (curOscPath, curSegment), r);
                    return;
                }

                throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No matching state object for OSC path at ") += osc::Util::appendPathSegment (curOscPath, curSegment));
            }

            // Wildcard match against member keys.
            auto didMatch = false;

            for (const auto& [key, value] : curState.items())
            {
                if (lo_pattern_match (key.c_str(), curSegment.c_str()))
                {
                    r (curSegmentIt, value, osc::Util::appendPathSegment (curOscPath, key), r);
                    didMatch = true;
                }
            }

            if (! didMatch)
                throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No matching state object for OSC path wildcard at ") += osc::Util::appendPathSegment (curOscPath, curSegment));

            return;
        }

        // JSON array: match OSC segment against nested arrays' indices.
        if (curState.is_array())
        {
            // No wildcard pattern: direct index match?
            if (! lo_string_contains_pattern (curSegment.c_str()))
            {
                if (const auto sub = findSubArrayForIndex (curState, string2index (curSegment)); sub != curState.end())
                {
                    r (curSegmentIt, (*sub)[1], osc::Util::appendPathSegment (curOscPath, curSegment), r);
                    return;
                }

                throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No matching state array member for index in OSC path at ") += osc::Util::appendPathSegment (curOscPath, curSegment));
            }

            // Wildcard match against index strings of inner arrays.
            auto subArrayMatchesWildcard = [&pattern = curSegment] (const auto& sub) {
                if (! sub.is_array() || sub.empty() || ! sub[0].is_number_unsigned())
                    throw ProdigyJsonCorruptStateException ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: State array does not contain indexed sub-arrays.");

                const auto index = nlohmann::to_string (sub[0]);
                return lo_pattern_match (index.c_str(), pattern.c_str());
            };

            auto didMatch = false;

            std::ranges::for_each (curState | std::views::filter (std::move (subArrayMatchesWildcard)), [&] (auto& sub) {
                r (curSegmentIt, sub[1], osc::Util::appendPathSegment (curOscPath, nlohmann::to_string (sub[0])), r);
                didMatch = true;
            });

            if (! didMatch)
                throw ProdigyJsonStateNotFoundException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: No matching state array member for OSC path wildcard at ") += osc::Util::appendPathSegment (curOscPath, curSegment));

            return;
        }

        // Unknown JSON value type in state tree.
        throw ProdigyJsonCorruptStateException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: Unsupported JSON value type in state at ") += curOscPath);
    };

    if (! matrix.contains (top))
        throw ProdigyJsonCorruptStateException (std::string ("MatrixOscAgent::forEachPrimitiveMatchingOscPattern: Cannot access sub-state pointed to by top at ") += top.to_string());

    recurse (oscSplit.begin(), matrix.at (top), initialOscPath, recurse);
}

} // namespace mrlab::controller
