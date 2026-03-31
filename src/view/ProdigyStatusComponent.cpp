/*
    ProdigyStatusComponent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Frederik Siepe/sonible GmbH
 */

#include "ProdigyStatusComponent.h"

namespace mrlab::view
{

ProdigyStatusComponent::ProdigyStatusComponent (controller::MatrixController& matrixController)
    : connectButton ("ProdigyStatusComponent::connectButton")
{
    connectButton.setButtonText ("Connect");
    addAndMakeVisible (connectButton);

    connectButton.onClick = [&, this] {
        if (state == controller::MatrixController::State::connected ||
            state == controller::MatrixController::State::waitingToReconnect)
            matrixController.disconnect();
        else
            matrixController.connect();
    };
}

void ProdigyStatusComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::white);
    g.drawRect (getLocalBounds(), 1);

    g.drawText (juce::String ("Prodigy Matrix Status: ") + std::string (controller::MatrixController::StateDescription::get (state)), getLocalBounds().reduced (10), juce::Justification::centredLeft);

    const auto bullet = getLocalBounds().removeFromRight (getHeight()).withSizeKeepingCentre (bulletSize, bulletSize).toFloat();
    g.setColour (bulletColour);
    g.fillEllipse (bullet);
    g.setColour (juce::Colours::white);
    g.drawEllipse (bullet, 1.0f);
}

void ProdigyStatusComponent::resized()
{
    constexpr auto width = 4 * bulletSize;
    constexpr auto height = bulletSize;

    connectButton.setBounds (getLocalBounds().withSizeKeepingCentre (width, height).withRightX (getWidth() - 2 * bulletSize));
}

void ProdigyStatusComponent::stateChanged (controller::MatrixController&, controller::MatrixController::State newState)
{
    state = newState;
    bool connectButtonEnabled = true;
    juce::String connectButtonText = "Connect";

    switch (state)
    {
        case controller::MatrixController::State::init:
            bulletColour = juce::Colours::black;
            break;
        default:
        case controller::MatrixController::State::disconnected:
            bulletColour = juce::Colours::red;
            break;
        case controller::MatrixController::State::waitingToReconnect:
            bulletColour = juce::Colours::orange;
            connectButtonText = "Cancel";
            break;
        case controller::MatrixController::State::disconnecting:
        case controller::MatrixController::State::connecting:
            bulletColour = juce::Colours::yellow;
            connectButtonEnabled = false;
            break;
        case controller::MatrixController::State::connected:
            bulletColour = juce::Colours::green;
            connectButtonText = "Disconnect";
            break;
    }

    connectButton.setEnabled (connectButtonEnabled);
    connectButton.setButtonText (connectButtonText);

    repaint();
}

} // namespace mrlab::view
