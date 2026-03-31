/*
    ProdigyStatusComponent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Frederik Siepe/sonible GmbH
 */

#include "ProdigyStatusComponent.h"

namespace mrlab::view
{

void ProdigyStatusComponent::paint (juce::Graphics& g)
{
    juce::Colour colour;

    switch (state)
    {
        case controller::MatrixController::State::init:
            colour = juce::Colours::black;
            break;
        default:
        case controller::MatrixController::State::disconnected:
            colour = juce::Colours::red;
            break;
        case controller::MatrixController::State::waitingToReconnect:
            colour = juce::Colours::orange;
            break;
        case controller::MatrixController::State::disconnecting:
        case controller::MatrixController::State::connecting:
            colour = juce::Colours::yellow;
            break;
        case controller::MatrixController::State::connected:
            colour = juce::Colours::green;
            break;
    }

    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);

    g.drawText (juce::String ("Prodigy Matrix Status: ") + std::string (controller::MatrixController::StateDescription::get (state)), getLocalBounds().reduced (10), juce::Justification::centredLeft);

    const auto bullet = getLocalBounds().removeFromRight (getHeight()).withSizeKeepingCentre (20, 20).toFloat();
    g.setColour (colour);
    g.fillEllipse (bullet);
    g.setColour (juce::Colours::white);
    g.drawEllipse (bullet, 1.0f);
}

void ProdigyStatusComponent::stateChanged (controller::MatrixController&, controller::MatrixController::State newState)
{
    state = newState;
    repaint();
}

} // namespace mrlab::view
