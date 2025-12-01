/*
    MainComponent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <controller/AppController.h>

namespace mrlab::controller
{
class MainController;
}

namespace mrlab::view
{
class AppControlComponent;

//==============================================================================
class MainComponent : public juce::Component,
                      public controller::AppController::Listener
{
public:
    //==============================================================================
    MainComponent (controller::MainController& controller);
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void appAdded (controller::AppHandle& app) override;
    void appWillBeRemoved (controller::AppHandle& app) override;

private:
    //==============================================================================
    controller::MainController& mainController;

    std::vector<std::unique_ptr<AppControlComponent>> appControls;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

} // namespace mrlab::view
