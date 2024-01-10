/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logoutobserver.h"

#include "app.h"
#include "utilities/leakdetector.h"

LogoutObserver::LogoutObserver(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(LogoutObserver);

  connect(App::instance(), &App::userStateChanged, this,
          &LogoutObserver::userStateChanged);
}

LogoutObserver::~LogoutObserver() { MZ_COUNT_DTOR(LogoutObserver); }

void LogoutObserver::userStateChanged() {
  if (App::instance()->userState() == App::UserNotAuthenticated) {
    emit ready();
    deleteLater();
  }
}
