/*
    AppConfigController.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppConfigController.h"

namespace mrlab::controller
{

AppConfigController::AppConfigController()
{}

AppConfig AppConfigController::findConfig (const juce::Identifier& appId) const
{
    if (appId == configFly)
    {
        return {
            .id = configFly,
            .name = "Pd Fly",
            .description = "Pd with Fly demonstration",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\PD\\YAMI\\FLY.bat"),
            .stopCommand = juce::StringArray ("C:\\Windows\\System32\\taskkill", "/IM", "pd.com", "/T", "/F"),
            .workingDir = juce::File ("C:\\PD\\YAMI")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/Pd-0.56-2.app/Contents/MacOS/Pd",
                                               "/Users/rm/Documents/Pd/test_stdout.pd"),
            .stopCommand = juce::StringArray ("killall", "Pd"),
            .workingDir = juce::File ("/Users/rm/Documents/Pd")
#endif
        };
    }

    if (appId == configReverb)
    {
        return {
            .id = configReverb,
            .name = "Reaper Reverb",
            .description = "Reaper DAW with artificial reverb",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\Program Files\\REAPER (x64)\\reaper",
                                               "reverb.rpp")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/REAPER.app/Contents/MacOS/REAPER",
                                               "-new")
#endif
        };
    }

    if (appId == configJungle)
    {
        return {
            .id = configJungle,
            .name = "Pd Jungle",
            .description = "Pd with Jungle demonstration",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\PD\\YAMI\\JUNGLE.bat"),
            .stopCommand = juce::StringArray ("C:\\Windows\\System32\\taskkill", "/IM", "pd.com", "/T", "/F"),
            .workingDir = juce::File ("C:\\PD\\YAMI")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/Pd-0.56-2.app/Contents/MacOS/Pd",
                                               "/Users/rm/Documents/Pd/test_stdout.pd"),
            .stopCommand = juce::StringArray ("killall", "Pd")
#endif
        };
    }

    if (appId == configMusic)
    {
        return {
            .id = configMusic,
            .name = "Reaper Music",
            .description = "Reaper DAW with music example",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\Program Files\\REAPER (x64)\\reaper",
                                               "music.rpp")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/REAPER.app/Contents/MacOS/REAPER",
                                               "-new")
#endif
        };
    }

    throw AppConfigNotFoundException (appId);
}

} // namespace mrlab::controller
