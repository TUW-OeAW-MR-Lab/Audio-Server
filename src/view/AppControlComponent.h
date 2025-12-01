/*
    AppControlComponent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <controller/AppHandle.h>


namespace mrlab::view
{

//==============================================================================
class AppControlComponent : public juce::Component,
                            public controller::AppHandle::Listener
{
public:
    //==============================================================================
    AppControlComponent (controller::AppHandle& appHandle);
    ~AppControlComponent() override;

    void appStateChanged (controller::AppHandle&, controller::AppHandle::AppState newState) override;
    void appOutputAvailable (controller::AppHandle&, const juce::String& newOutput) override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** @returns the id of the app represented by this. */
    juce::Identifier getId() const { return app.getConfig().id; }

private:
    //==============================================================================
    controller::AppHandle& app;

    juce::Label appIdLabel;
    juce::Label statusLabel;
    juce::TextButton launchQuitButton;
    juce::TextButton killButton;
    juce::TextButton openOutputButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppControlComponent)
};

} // namespace mrlab::view
