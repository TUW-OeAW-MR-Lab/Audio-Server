/*
    Address.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <string_view>

//==============================================================================
namespace mrlab::osc
{

/** Constants of OSC addresses (aka OSC paths) used for sending and receiving.

    @note This is not an exhaustive list of all OSC addresses being
    used as controllers may construct further addresses dynamically
    (e.g., app-specific ones).
 */
struct Address
{
    // general
    static constexpr std::string_view ping { "/ping" };
    static constexpr std::string_view pong { "/pong" };
    static constexpr std::string_view error { "/error" };

    // configmgr + config sub-path
    static constexpr std::string_view configmgr_list { "/configmgr/list" };
    static constexpr std::string_view configmgr_num { "/configmgr/num" };
    static constexpr std::string_view configmgr_control { "/configmgr/control" };
    static constexpr std::string_view config { "/config" };

    // app sub-path
    static constexpr std::string_view app { "/app" };

    // matrixmgr, matrixcmd and matrix sub-paths
    static constexpr std::string_view matrixmgr_state { "/matrixmgr/state" };
    static constexpr std::string_view matrixmgr_control { "/matrixmgr/control" };
    static constexpr std::string_view matrixcmd { "/matrixcmd" };
    static constexpr std::string_view matrix { "/matrix" };

    // totalmix sub-path
    static constexpr std::string_view totalmix { "/totalmix" };
};

} // namespace mrlab::osc
