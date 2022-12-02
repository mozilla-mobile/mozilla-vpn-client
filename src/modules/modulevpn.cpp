/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "leakdetector.h"

ModuleVPN::ModuleVPN(QObject* parent) : Module(parent, "vpn") {
  MVPN_COUNT_CTOR(ModuleVPN);
}

ModuleVPN::~ModuleVPN() { MVPN_COUNT_DTOR(ModuleVPN); }
