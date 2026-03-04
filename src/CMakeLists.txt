# src/CMakeLists.txt

# Part of MR Lab Control Software

# SPDX-License-Identifier: EUPL-1.2
# SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH

target_sources (mrlabctrl_App
    PRIVATE
        Exceptions.h
        Globals.h
        Globals.cpp
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
        controller/OscAgent.h
        controller/OscController.h
        controller/OscController.cpp
        controller/OscEndpoint.h
        controller/UdpEndpoint.h
        controller/UdpEndpoint.cpp
        controller/WebServerController.h
        controller/WebServerController.cpp
        controller/WebSocketController.h
        controller/WebSocketController.cpp
        controller/YamlConfig.h
        controller/YamlConfig.cpp

        osc/Address.h
        osc/Error.h
        osc/Message.h
        osc/Util.h

        util/ListenerInterface.h
        util/Logger.h

        view/AppControlComponent.h
        view/AppControlComponent.cpp
        view/LoggingComponent.h
        view/LoggingComponent.cpp
        view/MainComponent.h
        view/MainComponent.cpp
        view/MainWindow.h
        view/MainWindow.cpp
)
