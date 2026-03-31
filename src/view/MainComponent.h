/*
    MainComponent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "LoggingComponent.h"
#include "ProdigyStatusComponent.h"

namespace mrlab::controller
{
class MainController;
}

namespace mrlab::view
{

//==============================================================================
class MainComponent : public juce::Component
{
public:
    //==============================================================================
    MainComponent (controller::MainController& controller);
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    controller::MainController& mainController;

    LoggingComponent loggingComponent;
    ProdigyStatusComponent prodigyStatus;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

} // namespace mrlab::view
