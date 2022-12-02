/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulevpn.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"

#include <QQmlEngine>

namespace {
ModuleVPN* s_instance = nullptr;
}

ModuleVPN::ModuleVPN(QObject* parent) : Module(parent) {
  MVPN_COUNT_CTOR(ModuleVPN);

  Q_ASSERT(!s_instance);
  s_instance = this;

  connect(&m_controller, &Controller::stateChanged,
          MozillaVPN::instance()->captivePortalDetection(),
          &CaptivePortalDetection::stateChanged);

  connect(&m_controller, &Controller::stateChanged, &m_connectionHealth,
          &ConnectionHealth::connectionStateChanged);

  connect(&m_connectionHealth, &ConnectionHealth::stabilityChanged,
          MozillaVPN::instance()->statusIcon(), &StatusIcon::refreshNeeded);

  connect(&m_connectionHealth, &ConnectionHealth::stabilityChanged,
          MozillaVPN::instance()->captivePortalDetection(),
          &CaptivePortalDetection::stateChanged);

  m_connectionBenchmark.initialize();
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

QJSValue ModuleVPN::connectionBenchmarkValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_connectionBenchmark;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&ConnectionBenchmark::staticMetaObject));
  return value;
}

QJSValue ModuleVPN::connectionHealthValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_connectionHealth;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(
      engine->newQMetaObject(&ConnectionHealth::staticMetaObject));
  return value;
}

QJSValue ModuleVPN::controllerValue() {
  QJSEngine* engine = QmlEngineHolder::instance()->engine();

  QObject* obj = &m_controller;
  QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);

  QJSValue value = engine->newQObject(obj);
  value.setPrototype(engine->newQMetaObject(&Controller::staticMetaObject));
  return value;
}
