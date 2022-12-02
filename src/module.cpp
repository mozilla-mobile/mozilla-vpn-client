/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include "leakdetector.h"
#include "moduleholder.h"
#include "modules/modulevpn.h"

#include <QCoreApplication>

// static
void Module::initialize() {
  ModuleHolder::instance()->registerModule(new ModuleVPN(qApp));
}

Module::Module(QObject* parent, const QString& name)
    : QObject(parent), m_name(name) {
  MVPN_COUNT_CTOR(Module);
}

Module::~Module() { MVPN_COUNT_DTOR(Module); }
