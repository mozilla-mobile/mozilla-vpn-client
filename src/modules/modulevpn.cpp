/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "leakdetector.h"
#include "qmlengineholder.h"

#include <QQmlEngine>

namespace {
ModuleVPN* s_instance = nullptr;
}

ModuleVPN::ModuleVPN(QObject* parent) : Module(parent) {
  MVPN_COUNT_CTOR(ModuleVPN);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

ModuleVPN::~ModuleVPN() {
  MVPN_COUNT_DTOR(ModuleVPN);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
ModuleVPN* ModuleVPN::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

QJSValue ModuleVPN::controllerValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_controller;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&Controller::staticMetaObject));
  return value;
}
