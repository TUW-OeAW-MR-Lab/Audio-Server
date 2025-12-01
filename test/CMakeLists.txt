# test/CMakeLists.txt

# Part of MR Lab Control Software

# SPDX-License-Identifier: EUPL-1.2
# SPDX-FileCopyrightText: Copyright (c) 2025 Martin Rumori/sonible GmbH

CPMAddPackage ("gh:catchorg/Catch2@3.11.0")

#====================================================================================
juce_add_console_app (mrlabctrl_UnitTest
    PRODUCT_NAME  mrlabctrl_UnitTest
    COMPANY_NAME  sonible
    BUNDLE_ID     com.sonible.mrlabctrl_unittest
)

add_dependencies (run_tests mrlabctrl_UnitTest)

set_target_properties (mrlabctrl_UnitTest
    PROPERTIES
        EXCLUDE_FROM_ALL TRUE
)

target_link_libraries (mrlabctrl_UnitTest
    PRIVATE
        Catch2::Catch2WithMain
        mrlabctrl_Common
)

target_sources (mrlabctrl_UnitTest
    PRIVATE
        AppControllerTest.cpp
)

# Add Catch2 extras to module path such that we can include them.
list (APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
include (Catch)

# Automatically add unit test cases as single tests.
catch_discover_tests (mrlabctrl_UnitTest)
