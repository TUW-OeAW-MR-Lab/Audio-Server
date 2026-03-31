/*
    MatrixControllerOscAgent.h

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#pragma once

#include "MatrixController.h"
#include "OscAgent.h"

namespace mrlab::controller
{

//==============================================================================
/** OSC agent for OSC control of the MatrixController.

    Implements the OSC interface for controlling the MatrixController
    itself (control/report connection state to the Prodigy Matrix).

    @note This does not communicate with the Prodigy matrix, this is
          done by MatrixOscAgent.
 */
class MatrixControllerOscAgent : public OscAgent,
                                 public MatrixController::Listener
{
public:
    //==============================================================================
    MatrixControllerOscAgent (OscController& oscController, MatrixController& matrixControllerIn);
    ~MatrixControllerOscAgent() override;

    // MatrixController::Listener interface
    void stateChanged (MatrixController&, MatrixController::State) override;

private:
    //==============================================================================
    /** OSC message sending helper for /matrixmgr/state. */
    void sendMatrixmgrState (OscEndpoint* destination = nullptr);

    /** Register OSC message handlers. */
    void addMethods();

    //==============================================================================
    MatrixController& matrixController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MatrixControllerOscAgent)
};

} // namespace mrlab::controller
