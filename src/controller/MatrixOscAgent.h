/*
    MatrixOscAgent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "MatrixController.h"
#include "OscAgent.h"
#include <juce_events/juce_events.h>
#include <nlohmann/json.hpp>

namespace mrlab::controller
{

//==============================================================================
/** OSC agent for MatrixController.

    Provides an OSC interface from/to the JSON state tree of the
    Prodigy audio matrix. The implementation focuses on a
    bi-directional interface to items in the "settings" subgraph of
    the Prodigy's JSON state, as well as providing read-only access to
    the other state sub-trees.
 */
class MatrixOscAgent : public OscAgent,
                       public MatrixController::Listener,
                       private juce::Timer
{
public:
    //==============================================================================
    MatrixOscAgent (OscController& oscControllerIn, MatrixController& matrixControllerIn);
    ~MatrixOscAgent() override;

    // MatrixController::Listener interface
    void stateChanged (MatrixController&, MatrixController::State state) override;

    /** Handle an incoming Prodigy JSON control message from the matrix.

        @param msg Prodigy JSON message to handle.
     */
    void handleMatrixMessage (nlohmann::json msg);

private:
    //==============================================================================
    static constexpr auto prodigyPingIntervalMs = 5000;

    /** Send a ping to Prodigy matrix and warn on pending messages. */
    void timerCallback() override;

    /** Handle an incoming GET-RESP JSON message from the matrix. */
    void handleGetRespMessage (nlohmann::json&& msg);

    /** Handle an incoming UPDATE JSON message from the matrix. */
    void handleUpdateMessage (nlohmann::json&& msg);

    /** Handle an incoming ERROR JSON message from the matrix. */
    void handleErrorMessage (nlohmann::json&& msg);

    /** Handle an incoming ACK JSON message from the matrix. */
    void handleAckMessage (nlohmann::json&& msg);

    /** Create a Prodigy JSON control message for sending.

        This creates JSON messages for the Prodigy matrix control
        according to the Prodigy JSON procotol specification 1.0.

        @param type JSON control message type identifier.
        @param payload JSON value representing the message payload field.
        @param obj JSON value representing the optional obj pointer field.

        @return JSON value representing the Prodigy control message.

        @note The message will be assigned a sequence number, which
              will be added to the set of pending messages. It should
              be removed from the set when receiving the corresponding
              response (i.e., 'get_resp', 'ack' or 'error') using
              removeMessageFromPending().

        @see Type
     */
    nlohmann::json makeMessage (std::string_view type, const nlohmann::json& payload = {}, const nlohmann::json& obj = {});

    /** Remove a Prodigy JSON control message from the set of pending messages.

        If the input message contains a 'seq' field, the corresponding
        sequence number will be removed from the set of pending
        messages.

        @param msg Message whose sequence number should be removed from pending.

        @return true on success, false if no sequence number in message
                or sequence number not in set of pending messages.
     */
    bool removeMessageFromPending (const nlohmann::json& msg);

    /** Broadcast a state change message via all registered OSC endpoints.

        @param path OSC path to use for broadcasting.
        @param state Reference to the JSON state value to be used as an argument.
        @param origin Optional source endpoint of the message.

        @return true on sucess, false if the state value could not be converted.

        @note In case of failure to convert the JSON value, i.e., when
              false will be returned, an error will be logged.

        @see OscAgent::broadcast
     */
    bool broadcastStateChange (std::string_view path, const nlohmann::json& state, const OscEndpoint* origin = nullptr) const;

    /** Add one argument to an OSC message from a JSON value.

        @param osc OSC message to add the value to.
        @param json Input JSON value.
        @return true on sucess, false if the value was not added (e.g., unsupported).
     */
    static bool addJson2osc (osc::Message& osc, const nlohmann::json& json);

    /** Assign an OSC message argument to a JSON value.

        @param json JSON value to assign to.
        @param type OSC type tag of the OSC message argument to assign.
        @param osc OSC message argument to assign.
        @return true on sucess, false if the value was not assigned (e.g., unsupported).
     */
    static bool osc2json (nlohmann::json& json, char type, lo_arg* osc);

    /** Construct a Prodigy JSON state tree pointer from an OSC path.

        Splits the segments of the input OSC path and constructs an
        obj state pointer according to the Prodigy JSON procotol
        specification 1.0, section 2.2.8.2. To be used in Prodigy JSON
        messages like 'set' or 'get'.

        @param oscPath Input OSC path.
        @return JSON array representing the obj pointer.
     */
    static nlohmann::json oscPath2objPointer (std::string_view oscPath);

    /** Create an OSC path from a JSON pointer into the Prodigy matrix state.

        This will convert the JSON pointer to a string and prepend the
        /matrix prefix.

        @param pointer JSON pointer into the matrix state.
        @return String representing the OSC path corresponding to pointer.
     */
    static std::string matrixOscPathFromJsonPointer (const nlohmann::json::json_pointer& pointer);

    /** Parse string to array index.

        Parses the given string to a index (for array indexing). If
        parsing fails, a ProdigyJsonNoArrayIndexException will the
        thrown.

        @param string String containing the representation of the array index.
        @return On success, the resulting index.

       @throws ProdigyJsonNoArrayIndexException
     */
    static uint32_t string2index (std::string_view string);

    /** Find the Prodigy JSON state sub-array corresponding to a given index.

        Iterates through the members of a JSON array in order to find
        the sub-array containing the state that is referenced by
        index. This implements the Prodigy state array handling
        convention as described in Prodigy JSON procotol specification
        1.0, section 2.2.8.1. In case the given array does not contain
        indexed sub-arrays, an exception is thrown.

       @param array Outer JSON array containing the indexed sub-arrays.
       @param index Index whose corresponding sub-array to find.
       @return JSON iterator into array pointing at the respective
               sub-array or array.end() if not found.

       @throws ProdigyJsonCorruptStateException
     */
    static nlohmann::json::iterator findSubArrayForIndex (nlohmann::json& array, uint32_t index);

    /** Add an OSC method that will retrieve a value from the Prodigy JSON state.

        The OSC method will listen to any incoming OSC messages whose
        address pattern (aka path) can be matched against the JSON
        state tree and respond with an OSC message back to the source
        endpoint containing the current state as an argument. The
        method will cover the entire subtree of the Prodigy matrix
        state starting at the given node.

        @param top Pointer into the JSON state indicating the
               top-level node to add the method handler for.
     */
    void addStateGetterOscMethod (nlohmann::json::json_pointer top);

    /** Add an OSC method that will set a value in the Prodigy JSON state.

        The OSC method will listen to any incoming OSC messages whose
        address pattern (aka path) can be matched against the JSON
        state tree and whose argument is type-compatible with the
        referenced JSON state value.  The JSON state value will be set
        according to the OSC message's argument and the updated state
        will be both communicated to the Prodigy matrix and
        broadcasted to the registered OSC endpoints. The method will
        cover the entire subtree of the Prodigy matrix state starting
        at the given node.

        @param top Pointer into the JSON state indicating the
               top-level node to add the method handler for.
     */
    void addStateSetterOscMethod (nlohmann::json::json_pointer top);

    /** Helper to invoke a function for primitive JSON values in the Prodigy state tree.

        Recursively traverses a given payload JSON tree (e.g., as
        received by a Prodigy JSON 'update' message) until a primitive
        JSON value is found (i.e., the leafs of the tree). The
        provided function is called for each primitive value with a
        const reference to that value in the input payload tree, a
        reference to the JSON value in the internally stored, full
        matrix state tree that corresponds to the input payload value,
        and the OSC path that may be used to address this value by OSC
        messages.

        @param func Function to be called on each primitive payload value.
        @param payload JSON value representing the payload tree.
        @param top JSON pointer into the internal matrix state tree
               that corresponds to the top-level of the payload tree
               (e.g., "/settings" in case payload only references the
               "settings" sub-tree of the matrix state).

        @throws ProdigyJsonProtocolException, ProdigyJsonCorruptStateException
     */
    template <std::invocable<const nlohmann::json&, nlohmann::json&, std::string_view> Func>
    void forEachPrimitiveInPayload (Func&& func, const nlohmann::json& payload, nlohmann::json::json_pointer top = nlohmann::json::json_pointer());

    /** Helper to invoke a function for primitive JSON values in the Prodigy state tree.

        Recursively traverses the internally stored Prodigy matrix
        JSON state tree by wildcard-matching each segment of the given
        OSC address pattern and executes the provided function for
        each matching primitive JSON value (i.e., leafs of the state
        tree). The function is called with a reference to the matching
        value in the state tree and the OSC path that may be used to
        address this value by OSC messages.

        @param func Function to be called on each primitive state value.
        @param oscPattern  JSON value representing the payload tree.
        @param top JSON pointer into the internal matrix state tree
               that should be used as the top-level for recursive
               traversal (e.g., "/settings" in case only the
               "settings" subtree should be matched). oscPattern is
               still expected to begin with the top-level segments,
               i.e., "/matrix/settings/..." in the given example.

        @throws ProdigyJsonStateNotFoundException, ProdigyJsonCorruptStateException
     */
    template <std::invocable<nlohmann::json&, std::string_view> Func>
    void forEachPrimitiveMatchingOscPattern (Func&& func, std::string_view oscPattern, nlohmann::json::json_pointer top = nlohmann::json::json_pointer());

    //==============================================================================
    /** Prodigy JSON control message fields. */
    struct Field
    {
        static constexpr std::string_view type = "type";
        static constexpr std::string_view obj = "obj";
        static constexpr std::string_view payload = "payload";
        static constexpr std::string_view seq = "seq";
    };

    /** Prodigy JSON control message types. */
    struct Type
    {
        static constexpr std::string_view set = "set";
        static constexpr std::string_view update = "update";
        static constexpr std::string_view get = "get";
        static constexpr std::string_view get_resp = "get_resp";
        static constexpr std::string_view cmd = "cmd";
        static constexpr std::string_view ack = "ack";
        static constexpr std::string_view error = "error";
    };

    /** Prodigy JSON control message commands. */
    struct Command
    {
        static constexpr std::string_view flash = "flash";
        static constexpr std::string_view ping = "ping";
        static constexpr std::string_view reset_labels = "reset_labels";
        static constexpr std::string_view enable_auto_update = "enable_auto_update";
        static constexpr std::string_view disable_auto_update = "disable_auto_update";
        static constexpr std::string_view start_snapshot_recall = "start_snapshot_recall";
        static constexpr std::string_view stop_snapshot_recall = "stop_snapshot_recall";
    };

    //==============================================================================
    OscController& oscController;
    MatrixController& matrixController;
    nlohmann::json matrix;                   ///< Full state tree of the prodigy matrix.
    uint32_t sequence { 0 };                 ///< Next available message sequence number.
    std::unordered_set<uint32_t> pendingMsg; ///< Sequence numbers of pending messages.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatrixOscAgent)
};

} // namespace mrlab::controller
