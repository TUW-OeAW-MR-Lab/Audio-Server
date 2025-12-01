/*
    MainWindow.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include "MainWindow.h"
#include "MainComponent.h"

namespace mrlab::view
{

MainWindow::MainWindow (const juce::String& name, controller::MainController& mainController)
    : DocumentWindow (name, juce::Colours::lightgrey, DocumentWindow::allButtons),
      mainComponent (mainController)
{
    setUsingNativeTitleBar (true);
    setContentNonOwned (&mainComponent, true);

    setResizable (true, true);
    centreWithSize (getWidth(), getHeight());

    setVisible (true);
}

void MainWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

} // namespace mrlab::view
