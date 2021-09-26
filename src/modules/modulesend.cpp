/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulesend.h"
#include "features/featuremodulesend.h"

ModuleSend::ModuleSend()
    : Module("send", FEATURE_MODULE_SEND, "qrc:/ui/modules/Send.qml",
             "qrc:/ui/modules/SendWidget.qml") {}

static Module::RegistrationProxy<ModuleSend> s_moduleSend;
