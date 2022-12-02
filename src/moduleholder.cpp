/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "moduleholder.h"
#include "leakdetector.h"
#include "module.h"

#include <QCoreApplication>

// static
ModuleHolder* ModuleHolder::instance() {
  static ModuleHolder* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new ModuleHolder(qApp);
  }
  return s_instance;
}

ModuleHolder::ModuleHolder(QObject* parent) : QQmlPropertyMap(parent) {
  MVPN_COUNT_CTOR(ModuleHolder);
}

ModuleHolder::~ModuleHolder() { MVPN_COUNT_DTOR(ModuleHolder); }

void ModuleHolder::registerModule(const QString& moduleName,
                                  Module* moduleObj) {
  Q_ASSERT(!m_modules.contains(moduleName));
  m_modules.insert(moduleName, moduleObj);
  insert(moduleName, QVariant::fromValue(moduleObj));
}
