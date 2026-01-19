/*
    AppControlComponent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "AppControlComponent.h"
#include <util/Logger.h>

namespace mrlab::view
{

AppControlComponent::AppControlComponent (controller::AppHandle& appHandle)
    : app (appHandle),
      appIdLabel ("AppControlComponent::appIdLabel", app.getConfig().id.toString()),
      statusLabel ("AppControlComponent::statusLabel"),
      launchQuitButton ("AppControlComponent::launchQuitButton"),
      killButton ("AppControlComponent::killButton"),
      openOutputButton ("AppControlComponent::openOutputButton")
{
    killButton.setButtonText ("kill");
    openOutputButton.setButtonText ("output");

    launchQuitButton.setClickingTogglesState (true);
    openOutputButton.setEnabled (false);

    launchQuitButton.onClick = [&] {
        launchQuitButton.getToggleState() ? app.start() : app.stop();
    };

    killButton.onClick = [&] { app.kill(); };

    // TODO: Implement app output to logging (IMRV-38).
    // Just post the app output to stdout on button click for now.
    openOutputButton.onClick = [&] {
        Logger::logInfo (juce::String ("OpenOutputButton.onClick(): ") + app.getOutput());
        openOutputButton.setEnabled (false);
    };

    addAndMakeVisible (appIdLabel);
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (launchQuitButton);
    addAndMakeVisible (killButton);
    addAndMakeVisible (openOutputButton);

    app.addListener (this);
    appStateChanged (app, app.getState());
}

AppControlComponent::~AppControlComponent()
{
    app.removeListener (this);
}

void AppControlComponent::appStateChanged (controller::AppHandle&, controller::AppHandle::AppState newState)
{
    launchQuitButton.setToggleState (app.isRunning(), juce::dontSendNotification);
    launchQuitButton.setButtonText (app.isRunning() ? "quit" : "launch");
    launchQuitButton.setEnabled (newState != controller::AppHandle::AppState::alive);

    killButton.setEnabled (app.isRunning());

    auto statusText = controller::AppHandle::appStateNames.at (newState);

    if (app.isFinished())
        statusText += " (" + juce::String (app.getExitCode()) + ")";

    statusLabel.setText (statusText, juce::dontSendNotification);
}

void AppControlComponent::appOutputAvailable (controller::AppHandle&, const juce::String&)
{
    openOutputButton.setEnabled (true);

    // TODO: Implement app output to logging (IMRV-38).
}

void AppControlComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AppControlComponent::resized()
{
    const auto h = getHeight();
    const auto bw = 3 * h;
    const auto lw = juce::jmax (2 * h, (getWidth() - 3 * bw) / 2);

    appIdLabel.setBounds (0, 0, lw, h);
    launchQuitButton.setBounds (appIdLabel.getRight(), 0, bw, h);
    statusLabel.setBounds (launchQuitButton.getRight(), 0, lw, h);
    killButton.setBounds (statusLabel.getRight(), 0, bw, h);
    openOutputButton.setBounds (killButton.getRight(), 0, bw, h);
}

} // namespace mrlab::view
