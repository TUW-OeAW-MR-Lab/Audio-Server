/*
    Config.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <JuceHeader.h>

namespace mrlab
{

//==============================================================================
/** Container for static configuration data.

    Collects hardcoded configuration data for now, might be extended
    by a configuration file reader.
 */
struct Config
{
    //==============================================================================
    /** @return The general application support base directory. */
    static juce::File getAppSupportBaseDir()
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

    /** @return The base directory of this application's data and associated resources. */
    static juce::File getAppSupportDir()
    {
        return getAppSupportBaseDir().getChildFile (ProjectInfo::projectName);
    }

    /** @return The web server's document root (holds web gui data). */
    static juce::File getWebServerDocumentRootDir()
    {
        return getAppSupportDir().getChildFile ("WebGUI");
    }

    /** @return The directory containing scene/app configurations. */
    static juce::File getSceneConfigDir()
    {
        return getAppSupportDir().getChildFile ("Config");
    }

    //==============================================================================
    /** @return The listening port for the webserver (serving the web gui). */
    static uint16_t getWebServerListeningPort() { return 7080; }

    /** @return The listening port for 3rd party OSC control. */
    static uint16_t getOscListeningPort() { return 7081; }
};

} // namespace mrlab
