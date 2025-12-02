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
{
}

AppConfig AppConfigController::findConfig (const juce::Identifier& appId) const
{
    // Reaper test config
    if (appId == testConfig0)
    {
        return {
            .id = testConfig0,
            .name = "Reaper Empty 0",
            .description = "Reaper DAW with empty test session",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("C:\\Program Files\\REAPER (x64)\\reaper",
                                               "-new")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/REAPER.app/Contents/MacOS/REAPER",
                                               "-new")
#endif
        };
    }

    if (appId == testConfig1)
    {
        return {
            .id = testConfig1,
            .name = "Pd YAMI 1",
            .description = "Pd with YAMI test patch",
#if JUCE_WINDOWS
            .startCommand = juce::StringArray ("F:\\YAMI_20250311\\YAMI\\My YAMI2.bat"),
            .workingDir = juce::File ("F:\\YAMI_20250311\\YAMI")

            //.startCommand = juce::StringArray ("C:\\Program Files\\Pd\\bin\\Pd",
            //                                   "C:\\Users\\fux\\Documents\\Pd\\test_stdout.pd")
#elif JUCE_MAC
            .startCommand = juce::StringArray ("/Applications/Pd-0.56-2.app/Contents/MacOS/Pd",
                                               "/Users/rm/Documents/Pd/test_stdout.pd"),
            .stopCommand = juce::StringArray ("killall", "Pd")
#endif
        };
    }

    throw AppConfigNotFoundException (appId);
}

} // namespace mrlab::controller
