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

App::UserState App::userState() const { return m_userState; }

void App::setState(int state) {
  logger.debug() << "Set state:" << state;

  m_state = state;
  emit stateChanged();
}

void App::setUserState(UserState state) {
  logger.debug() << "User authentication state:" << state;
  if (m_userState != state) {
    m_userState = state;
    emit userStateChanged();
  }
}

void App::quit() {
  logger.debug() << "quit";
  TaskScheduler::forceDeleteTasks();

#if QT_VERSION < 0x060300
  // Qt5Compat.GraphicalEffects makes the app crash on shutdown. Let's do a
  // quick exit. See: https://bugreports.qt.io/browse/QTBUG-100687

  SettingsManager::instance()->sync();
  exit(0);
#endif

  qApp->quit();
}
