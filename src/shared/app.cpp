/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "app.h"

#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

namespace {
Logger logger("App");
}

App::App(QObject* parent) : QObject(parent) { MZ_COUNT_CTOR(App); }

App::~App() { MZ_COUNT_DTOR(App); }

App::UserState App::userState() const { return m_userState; }

void App::setUserState(UserState state) {
  logger.debug() << "User authentication state:" << state;
  if (m_userState != state) {
    m_userState = state;
    emit userStateChanged();
  }
}

// static
QByteArray App::authorizationHeader() {
  if (SettingsHolder::instance()->token().isEmpty()) {
    logger.error() << "INVALID TOKEN! This network request is going to fail.";
    Q_ASSERT(false);
  }

  QByteArray authorizationHeader = "Bearer ";
  authorizationHeader.append(SettingsHolder::instance()->token().toLocal8Bit());
  return authorizationHeader;
}
