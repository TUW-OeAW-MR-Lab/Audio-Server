/*
    SupportFileLocation.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_core/juce_core.h>

#if JUCE_WINDOWS
#define APP_SUPPORT_DIR juce::File (juce::SystemStats::getEnvironmentVariable ("APPDATA", juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getFullPathName())).getChildFile ("mrlabctrl")
#elif JUCE_MAC
#define APP_SUPPORT_DIR juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Application Support/mrlabctrl")
#elif JUCE_LINUX
#define APP_SUPPORT_DIR juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".local/share/mrlabctrl")
#else
#define APP_SUPPORT_DIR juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("mrlabctrl")
#endif