/*
    MainComponent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "MainComponent.h"
#include <controller/MainController.h>

namespace mrlab::view
{

MainComponent::MainComponent (controller::MainController& controller)
    : mainController (controller),
      loggingComponent (controller.getLogger()),
      prodigyStatus (mainController.getMatrixController())
{
    addAndMakeVisible (loggingComponent);
    addAndMakeVisible (prodigyStatus);

    mainController.getMatrixController().addListener (&prodigyStatus);

    setSize (800, 350);
}

MainComponent::~MainComponent()
{
    mainController.getMatrixController().removeListener (&prodigyStatus);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    loggingComponent.setBounds (getLocalBounds().withHeight (300).reduced (10));
    prodigyStatus.setBounds (getLocalBounds().removeFromBottom (50).reduced (10));
}

} // namespace mrlab::view
