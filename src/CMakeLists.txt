# src/CMakeLists.txt

# Part of MR Lab Control Software

# SPDX-License-Identifier: EUPL-1.2
# SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH

target_sources (mrlabctrl_App
    PRIVATE
        Main.cpp
)

target_sources (mrlabctrl_Common
    INTERFACE
        controller/AppConfigController.h
        controller/AppConfigController.cpp
        controller/AppController.h
        controller/AppController.cpp
        controller/AppHandle.h
        controller/AppHandle.cpp
        controller/MainController.h
        controller/MainController.cpp
        controller/OscController.h
        controller/OscController.cpp
        controller/WebServerController.h
        controller/WebServerController.cpp

        util/ListenerInterface.h

        view/AppControlComponent.h
        view/AppControlComponent.cpp
        view/MainComponent.h
        view/MainComponent.cpp
        view/MainWindow.h
        view/MainWindow.cpp
)
