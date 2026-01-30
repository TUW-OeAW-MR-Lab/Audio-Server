/*
    Main.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH
 */

#include <JuceHeader.h>

#include "controller/MainController.h"
#include "view/MainWindow.h"

namespace mrlab
{

//==============================================================================
class MrLabControlApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return false; }

    //==============================================================================
    void initialise (const juce::String& /*commandLine*/) override
    {
        mainController = std::make_unique<controller::MainController>();
        mainWindow = std::make_unique<view::MainWindow> (getApplicationName(), *mainController);

        // Load configuration files from app support directory.
        mainController->getConfigController().populateFromConfigDir();
    }

    //==============================================================================
    void shutdown() override
    {
        mainWindow.reset();
        mainController.reset();
    }

    void systemRequestedQuit() override
    {
        if (mainController->getAppController().isAnyAppRunning())
        {
            auto callWhenAppsStopped = [&] (bool timedOut) {

                if (timedOut)
                    mainController->getAppController().killAllApps();

                quit();
            };

            auto messageBoxCallback = [&, whenStopped = std::move (callWhenAppsStopped)] (bool result) {
                if (result)
                    mainController->getAppController().stopAllApps (std::move (whenStopped), 2000);
            };

            juce::NativeMessageBox::showOkCancelBox (juce::MessageBoxIconType::WarningIcon,
                                                     "Apps are running",
                                                     "There are still managed apps running. "
                                                     "Ok to quit all apps?",
                                                     mainWindow.get(),
                                                     juce::ModalCallbackFunction::create (std::move (messageBoxCallback)));
            return;
        }

        quit();
    }

    void anotherInstanceStarted (const juce::String& /*commandLine*/) override {}

private:
    //==============================================================================
    std::unique_ptr<controller::MainController> mainController;
    std::unique_ptr<view::MainWindow> mainWindow;
};

} // namespace mrlab

//==============================================================================
START_JUCE_APPLICATION (mrlab::MrLabControlApplication)
