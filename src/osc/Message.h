/*
    Message.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <lo/lo_cpp.h>

namespace mrlab::osc
{

//==============================================================================
/** OSC message class specialisation fixing some shortcomings of lo::Message. */
class Message : public lo::Message
{
public:
    //==============================================================================
    using lo::Message::add;

    /** Variadic template constructor.

        This facilitates the construction of OSC messages with
        primitive argument types without the necessity to specify a
        type string.
    */
    template <class... Args>
    Message (Args... args)
    {
        (add (std::forward<Args> (args)), ...);
    }

    /** Overload for adding a string argument.

       This is missing from lo::Message because it conflicts with the
       base type's C style variadic function
       @code add (const string_type types, ...) @endcode

       For the time being, using a std::string_view for the string
       argument overload seems to cheat overload resolution, so if the
       compiler complains about an ambiguous call, passing a
       std::string_view might work.
    */
    int add (std::string_view a)
    {
        return add_string (a);
    }
};

} // namespace mrlab::osc
