/*
    Util.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include <string_view>
#include <ranges>
#include <lo/lo_cpp.h>

//==============================================================================
namespace mrlab::osc
{

/** OSC protocol utility functions. */
struct Util
{
    /** Helper to syntactically check an OSC path.

        @param path OSC path to check.
        @param allowEmpty Flag whether an empty path is regarded as valid.
     */
    static constexpr bool validateOscPath (std::string_view path, bool allowEmpty = false)
    {
        if (allowEmpty && path.empty())
            return true;

        return path.starts_with ('/') && ! path.ends_with ('/') && path.size() > 1;
    }

    /** Helper to retrieve the number of segments in an OSC path.

        @param path Input OSC path.

        @return The number of path segments in the OSC path.

        @note This function does not perform OSC path validation, so
              ill-formed paths will likely produce unexpected results.
     */
    static constexpr size_t getNumPathSegments (std::string_view path)
    {
        using namespace std::string_view_literals;

        // Remove leading '/' before splitting.
        path.remove_prefix (1);
        auto split = std::views::split (path, "/"sv);
        return size_t (std::ranges::distance (split));
    }

    /** Helper to retrieve the nth segment of an OSC path.

        @param path Input OSC path.
        @param n Number of the segment to extract (starting from 0).

        @return The extracted path segment or an empty
                std::string_view if n is greater than the number of
                segments in path.

        @note This function does not perform OSC path validation, so
              ill-formed paths will likely produce unexpected results.
     */
    static constexpr std::string_view getPathSegment (std::string_view path, size_t n)
    {
        using namespace std::string_view_literals;

        // Remove leading '/' before splitting.
        path.remove_prefix (1);
        auto split = std::views::split (path, "/"sv);
        auto segment = split.begin();

        while (n--)
        {
            if (++segment == split.end())
                return std::string_view();
        }

        // std::string_view constructor from std::subrange only coming in C++-23.
        return std::string_view ((*segment).data(), (*segment).size());
    }

    /** Helper to append a segment to an OSC path.

        @param path Input OSC path.
        @param segment Path segment to append (without a leading '/').

        @return The resulting OSC path.

        @note This function does not perform OSC path validation, so
              ill-formed paths will likely produce unexpected results.
     */
    static std::string appendPathSegment (std::string_view path, std::string_view segment)
    {
        std::string newPath { path };
        (newPath += '/') += segment;
        return newPath;
    }

    /** Helper to format a liblo server error callback message.

        @param prefix Prefix to prepend the log message.
        @param num liblo error number.
        @param msg liblo error message.
        @param wheher liblo error location.

        @return A formatted error string suitable, e.g., for logging.
     */
    static juce::String formatServerError (const lo::Server& server, juce::StringRef prefix, int num, const char* msg, const char* where)
    {
        const auto port = server.is_valid() ? server.port() : -1;
        auto result = juce::String (prefix);

        (((result += "lo::Server error (port ") += juce::String (port)) += "): ") += juce::String (num);

        if (msg)
            (result += " (") += msg;
        if (where)
            (result += "; ") += where;

        return result += ").";
    }
};

} // namespace mrlab::osc
