/*
    MainComponent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "MainComponent.h"
#include "AppControlComponent.h"
#include <controller/MainController.h>
#include <controller/AppHandle.h>


namespace mrlab::view
{

MainComponent::MainComponent (controller::MainController& controller)
    : mainController (controller)
{
    mainController.getAppController().addListener (this);

    setSize (800, 300);
}

MainComponent::~MainComponent()
{
    mainController.getAppController().removeListener (this);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto y = 0;

    for (auto& control : appControls)
        control->setBounds (50, y += 50, getWidth() - 100, 35);
}

void MainComponent::appAdded (controller::AppHandle& app)
{
    auto& c = appControls.emplace_back (std::make_unique<AppControlComponent> (app));
    addAndMakeVisible (*c);
    resized();
}

void MainComponent::appWillBeRemoved (controller::AppHandle& app)
{
    const auto it = std::find_if (appControls.begin(), appControls.end(), [id = app.getConfig().id] (auto& c) { return c->getId() == id; });

    if (it == appControls.end())
        return;

    removeChildComponent (it->get());
    appControls.erase (it);
    resized();
}

} // namespace mrlab::view
