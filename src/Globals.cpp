/*
    Globals.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "Globals.h"
#include <JuceHeader.h>

namespace mrlab
{

juce::File Globals::getAppSupportBaseDir()
{
#if JUCE_WINDOWS
    return { juce::SystemStats::getEnvironmentVariable ("PROGRAMDATA", juce::File::getSpecialLocation (juce::File::commonApplicationDataDirectory).getFullPathName()) };
#elif JUCE_MAC
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Application Support");
#elif JUCE_LINUX
    return juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".local/share");
#else
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
#endif
}

juce::File Globals::getAppSupportDir()
{
    return getAppSupportBaseDir().getChildFile (ProjectInfo::projectName);
}

juce::File Globals::getLogFile()
{
    return getAppSupportDir().getChildFile ("MRLabLog.txt");
}

} // namespace mrlab
