/*
    AppConfigController.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>

namespace mrlab::controller
{

//==============================================================================
/** Preliminary incomplete app config record for testing.

    To be outsourced to a separate file and extended by file reading capabilities.
 */
struct AppConfig
{
    juce::Identifier id;      ///< Unique app identifier.
    juce::String name;        ///< Short name to be displayed in user interfaces.
    juce::String description; ///< Detailed app description.

    juce::StringArray startCommand; ///< Command line to execute for launching (executable and arguments).
    juce::StringArray stopCommand;  ///< Command line to execute for quitting (if empty, kill will be used).

    juce::File workingDir;          ///< Working directory for app launching.

    bool captureStdOut = true;      ///< Flag to indicate whether to capture the app's std output.
    bool captureStdErr = true;      ///< Flag to indicate whether to capture the app's std error.
};


//==============================================================================
/** Manage and provide app configurations from config files. */
class AppConfigController
{
public:
    //==============================================================================
    /** Exception that is thrown when there is no app for the given id. */
    class AppConfigNotFoundException : public std::exception
    {
    public:
        AppConfigNotFoundException (const juce::Identifier& appId)
            : msg ("AppConfigNotFoundException: no app config found for id " + appId.toString())
        {}

        const char* what() const noexcept override { return msg.toUTF8(); }

    protected:
        juce::String msg;
    };

    //==============================================================================
    AppConfigController();

    // Temporary test config ids
    inline static const auto configFly = juce::Identifier ("pd-fly");		
    inline static const auto configReverb = juce::Identifier ("reaper-reverb");
    inline static const auto configJungle = juce::Identifier ("pd-jungle");
    inline static const auto configMusic = juce::Identifier ("reaper-music");

    /** Get the app configuration for appId.

        @returns AppConfig for appId.
        @throws AppConfigNotFoundException.
     */
    AppConfig findConfig (const juce::Identifier& appId) const;

private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppConfigController)
};

} // namespace mrlab::controller
