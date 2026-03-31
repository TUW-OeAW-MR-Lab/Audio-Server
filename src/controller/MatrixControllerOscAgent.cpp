/*
    MatrixControllerOscAgent.cpp

    Part of MR Lab Control Software

    SPDX-License-Identifier: EUPL-1.2
    SPDX-FileCopyrightText: Copyright (c) 2026 Martin Rumori/sonible GmbH
 */

#include "MatrixControllerOscAgent.h"

namespace mrlab::controller
{

MatrixControllerOscAgent::MatrixControllerOscAgent (OscController& oscController, MatrixController& matrixControllerIn)
    : OscAgent (oscController),
      matrixController (matrixControllerIn)
{
    matrixController.addListener (this);
    addMethods();
}

MatrixControllerOscAgent::~MatrixControllerOscAgent()
{
    matrixController.removeListener (this);
}

void MatrixControllerOscAgent::stateChanged (MatrixController&, MatrixController::State)
{
    sendMatrixmgrState();
}

void MatrixControllerOscAgent::sendMatrixmgrState (OscEndpoint* destination)
{
    auto state = matrixController.getState();
    auto msg = osc::Message (int32_t (state), MatrixController::StateDescription::get (state));

    sendOrBroadcast (destination, osc::Address::matrixmgr_state, msg);
}

void MatrixControllerOscAgent::addMethods()
{
    // Query the controller connection state.
    addMethod (osc::Address::matrixmgr_state, {}, [this] (OscEndpoint* source) {
        sendMatrixmgrState (source);
    });

    // Control (matrix connect/disconnect).
    addMethod (osc::Address::matrixmgr_control, "s", [this] (OscEndpoint* source, lo_arg** argv, int /*argc*/) {
        std::string_view command = &argv[0]->s;

        if (command == "connect")
            matrixController.connect();
        else if (command == "disconnect")
            matrixController.disconnect();
        else
            sendError (source, osc::Error::matrixMgrCommandUnknown, command);
    });
}

} // namespace mrlab::controller
