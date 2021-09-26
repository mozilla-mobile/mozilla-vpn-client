/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "features/featuremodulevpn.h"

ModuleVPN::ModuleVPN()
    : Module("vpn", FEATURE_MODULE_VPN, "qrc:/ui/modules/VPN.qml",
             "qrc:/ui/modules/VPNWidget.qml") {}

static Module::RegistrationProxy<ModuleVPN> s_moduleVPN;
