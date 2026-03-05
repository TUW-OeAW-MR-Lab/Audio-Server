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

    // /configinfo
    static constexpr std::string_view configinfo_list { "/configinfo/list" };
    static constexpr std::string_view configinfo_num { "/configinfo/num" };
};

} // namespace mrlab::osc
