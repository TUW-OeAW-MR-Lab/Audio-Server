/*
    MainWindow.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "MainComponent.h"

namespace mrlab::view
{

//==============================================================================
class MainWindow : public juce::DocumentWindow
{
public:
    //==============================================================================
    MainWindow (const juce::String& name, controller::MainController& mainController);

private:
    //==============================================================================
    MainComponent mainComponent;

    void closeButtonPressed() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

} // namespace mrlab::view
