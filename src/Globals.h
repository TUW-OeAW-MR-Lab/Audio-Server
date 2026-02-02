/*
    Globals.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>

namespace mrlab
{

//==============================================================================
/** Container for static global configuration data.

    Collects hardcoded configuration data for now, might be extended
    by a configuration file reader.
 */
struct Globals
{
    //==============================================================================
    /** @return The general application support base directory. */
    static juce::File getAppSupportBaseDir();

    /** @return The base directory of this application's data and associated resources. */
    static juce::File getAppSupportDir();

    /** @return The location and name of the log file to be written. */
    static juce::File getLogFile();

    /** @return The web server's document root (holds web gui data). */
    static juce::File getWebServerDocumentRootDir()
    {
        return getAppSupportDir().getChildFile ("WebGUI");
    }

    /** @return The directory containing configuration files (app/routing/...). */
    static juce::File getConfigDir()
    {
        return getAppSupportDir().getChildFile ("Config");
    }

    /** @return The file extension of configuration files (app/routing/...). */
    static juce::String getConfigFileExtension()
    {
      return "yaml";
    }

    //==============================================================================
    /** @return The listening port for the webserver (serving the web gui). */
    static uint16_t getWebServerListeningPort() { return 7080; }

    /** @return The listening port for 3rd party OSC control. */
    static uint16_t getOscListeningPort() { return 7081; }
};

} // namespace mrlab
