/*
    Error.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <string_view>

//==============================================================================
namespace mrlab::osc
{

/** Error codes used for sending OSC /error messages. */
enum class Error
{
    // general
    none = 0x00,
    generic,

    // config
    configIdUnknown = 0x10,
    configCommandUnknown,
    configReloadFailed,

    // app
    appCommandUnknown = 0x20,

};

/** Descriptive texts for OSC error codes. */
struct ErrorDescription
{
    static std::string_view get (Error code)
    {
        return description.at (code);
    }

    /** Error codes to description strings. */
    inline static const auto description = std::map<Error, std::string_view> (
        { { Error::none, "Success" },
          { Error::generic, "Generic error" },

          // config
          { Error::configIdUnknown, "Config id unknown" },
          { Error::configCommandUnknown, "Config control command unknown" },
          { Error::configReloadFailed, "Config reloading failed" },

          // app
          { Error::appCommandUnknown, "App control command unknown" },


        });
};

} // namespace mrlab::osc
