/*
    ConfigOscAgent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "ConfigController.h"
#include "OscAgent.h"

namespace mrlab::controller
{

//==============================================================================
/** OSC agent for ConfigController.

    Implements the OSC interface for config info retrieval and control
    (e.g., config reloading).
 */
class ConfigOscAgent : public OscAgent,
                       public ConfigController::Listener
{
public:
    //==============================================================================
    ConfigOscAgent (OscController& oscControllerIn, ConfigController& configControllerIn);
    ~ConfigOscAgent() override;

    // ConfigController::Listener interface
    void configAdded (const YamlConfig& config) override;
    void configWillBeRemoved (const YamlConfig& config) override;
    void configHasBeenRemoved (const juce::Identifier& id) override;

private:
    //==============================================================================
    /** OSC message sending helper for /configinfo/list. */
    void sendConfiginfoList (OscEndpoint* destination = nullptr);

    /** OSC message sending helper for /configinfo/num. */
    void sendConfiginfoNum (OscEndpoint* destination = nullptr);

    /** Register OSC message handlers. */
    void addMethods();

    //==============================================================================
    ConfigController& configController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigOscAgent)
};

} // namespace mrlab::controller
