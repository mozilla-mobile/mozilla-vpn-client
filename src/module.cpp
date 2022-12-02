/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include "leakdetector.h"
#include "moduleholder.h"
#include "modules/modulevpn.h"

#include <QCoreApplication>

#define MODULE(key, obj)
#include "modulelist.h"
#undef MODULE

// static
void Module::initialize() {
#define MODULE(key, obj) \
  ModuleHolder::instance()->registerModule(#key, new obj(qApp));
#include "modulelist.h"
#undef MODULE
}

Module::Module(QObject* parent) : QObject(parent) { MVPN_COUNT_CTOR(Module); }

Module::~Module() { MVPN_COUNT_DTOR(Module); }
