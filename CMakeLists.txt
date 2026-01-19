# CMakeLists.txt

# Part of MR Lab Control Software

# SPDX-License-Identifier: EUPL-1.2
# SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH

cmake_minimum_required (VERSION 3.22)

# This setting has to occur BEFORE the project call to work correctly
set (CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "macOS deployment target" FORCE)

project (mrlab VERSION 0.0.1)

# CPM dependency management: https://github.com/cpm-cmake/CPM.cmake
include (cmake/CPM.cmake)

# Compile commands, useful for some IDEs like VS-Code
set (CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Static linkage on Windows
set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Define alias for builtin test target such that we can add a dependency to it.
# Undocumented feature: https://gitlab.kitware.com/cmake/cmake/-/issues/22239
# This has to happen before including CTest and thus before adding the CivetWeb package
# because CivetWeb includes CTest even if CIVETWEB_BUILD_TESTING is switched off.
set (CTEST_TEST_TARGET_ALIAS run_tests)
include (CTest)

# JUCE dependency
CPMAddPackage ("gh:juce-framework/JUCE#8.0.12")

# yaml-cpp
set (CMAKE_POLICY_VERSION_MINIMUM "3.5")
CPMAddPackage (
    URI "gh:jbeder/yaml-cpp#0.8.0"
    OPTIONS
        "YAML_BUILD_SHARED_LIBS OFF"
        "YAML_CPP_BUILD_TESTS OFF"
        "YAML_CPP_BUILD_TOOLS OFF"
)
unset (CMAKE_POLICY_VERSION_MINIMUM)

# CivetWeb for webserver and websocket
CPMAddPackage (
    URI "gh:civetweb/civetweb#dfdfdb6926765541751cab055472f70daa9aa2dd"
    OPTIONS
    "CIVETWEB_ENABLE_SERVER_EXECUTABLE OFF"
    "CIVETWEB_BUILD_TESTING OFF"
    "CIVETWEB_ENABLE_ASAN OFF"
    "CIVETWEB_ENABLE_CXX ON"
    "CIVETWEB_ENABLE_WEBSOCKETS ON"
)

# liblo osc library
CPMAddPackage (
    URI "gh:radarsat1/liblo#0.34"
    SOURCE_SUBDIR "cmake"
    OPTIONS
    "WITH_TOOLS OFF"
    "WITH_TESTS OFF"
    "WITH_EXAMPLES OFF"
    "WITH_CPP_TESTS OFF"
    "WITH_STATIC ON"
)

#====================================================================================
juce_add_gui_app (mrlabctrl_App
    PRODUCT_NAME  MRLabAudioServer
    COMPANY_NAME  sonible
    BUNDLE_ID     com.sonible.mrlabctrl

    HARDENED_RUNTIME_ENABLED TRUE
)

# We do not intend to use JuceHeader.h generally,
# this is just for exploiting its ProjectInfo:: facility.
juce_generate_juce_header (mrlabctrl_App)

# Interface library for code shared between, e.g., the main and test targets.
add_library (mrlabctrl_Common INTERFACE)

add_subdirectory (resources)
add_subdirectory (src)
add_subdirectory (test)

target_include_directories (mrlabctrl_Common INTERFACE "src")

target_compile_definitions (mrlabctrl_Common
    INTERFACE
        DONT_SET_USING_JUCE_NAMESPACE=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        LO_USE_EXCEPTIONS=1
)

target_compile_features (mrlabctrl_Common
    INTERFACE
        cxx_std_20
)

target_link_libraries (mrlabctrl_Common
    INTERFACE
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        juce::juce_core
        juce::juce_gui_extra
        civetweb-cpp
        liblo_static
        yaml-cpp::yaml-cpp
)

target_link_libraries (mrlabctrl_App
    PRIVATE
        mrlabctrl_Common
)
