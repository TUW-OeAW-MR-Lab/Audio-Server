/*
    ProdigyStatusComponent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Frederik Siepe/sonible GmbH
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <controller/MatrixController.h>

namespace mrlab::view
{
//==============================================================================
class ProdigyStatusComponent : public juce::Component,
                               public controller::MatrixController::Listener
{
public:
    //==============================================================================
    ProdigyStatusComponent (controller::MatrixController& matrixController);

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

    void stateChanged (controller::MatrixController& controller, controller::MatrixController::State newState) override;

private:
    //==============================================================================
    static constexpr auto bulletSize = 20;

    //==============================================================================
    controller::MatrixController::State state = controller::MatrixController::State::init;
    juce::TextButton connectButton;
    juce::Colour bulletColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProdigyStatusComponent)
};

} // namespace mrlab::view
