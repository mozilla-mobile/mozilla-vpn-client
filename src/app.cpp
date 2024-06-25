/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "app.h"

#include <QCoreApplication>

#include "leakdetector.h"
#include "logger.h"
#include "settings/settingsmanager.h"
#include "settingsholder.h"
#include "taskscheduler.h"

#ifdef MZ_WASM
#  include "networkrequest.h"
#  include "platforms/wasm/wasmnetworkrequest.h"
#endif

namespace {
Logger logger("App");
}

App::App(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(App);

#ifdef MZ_WASM
  NetworkRequest::setRequestHandler(WasmNetworkRequest::deleteResource,
                                    WasmNetworkRequest::getResource,
                                    WasmNetworkRequest::postResource,
                                    WasmNetworkRequest::postResourceIODevice);
#endif
}

App::~App() { MZ_COUNT_DTOR(App); }

int App::state() const { return m_state; }

void App::setState(int state) {
  logger.debug() << "Set state:" << state;

  m_state = state;
  emit userAuthenticationMaybeChanged();
  emit stateChanged();
}

void App::quit() {
  logger.debug() << "quit";
  TaskScheduler::forceDeleteTasks();

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 4)
  // Qt5Compat.GraphicalEffects makes the app crash on shutdown. Let's do a
  // quick exit. See: https://bugreports.qt.io/browse/QTBUG-100687
  // it is unknown when exactly this was fixed but aparently 6.2.4 is fine.

  SettingsManager::instance()->sync();
  exit(0);
#endif

  qApp->quit();
}

bool App::userAuthenticated() const {
  auto holder = SettingsHolder::instance();
  return (m_state > StateAuthenticating) && holder->hasToken();
}
