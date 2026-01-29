# src/CMakeLists.txt

# Part of MR Lab Control Software

# SPDX-License-Identifier: EUPL-1.2
# SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH

target_sources (mrlabctrl_App
    PRIVATE
        Globals.h
        Main.cpp
)

target_sources (mrlabctrl_Common
    INTERFACE
        controller/AppController.h
        controller/AppController.cpp
        controller/AppHandle.h
        controller/AppHandle.cpp
        controller/ConfigController.h
        controller/ConfigController.cpp
        controller/MainController.h
        controller/MainController.cpp
        controller/OscController.h
        controller/OscController.cpp
        controller/WebServerController.h
        controller/WebServerController.cpp

        util/ListenerInterface.h
        util/Logger.h

        view/AppControlComponent.h
        view/AppControlComponent.cpp
        view/MainComponent.h
        view/MainComponent.cpp
        view/MainWindow.h
        view/MainWindow.cpp
)
