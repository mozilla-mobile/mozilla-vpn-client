/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logoutobserver.h"
#include "leakdetector.h"
#include "mozillavpn.h"

LogoutObserver::LogoutObserver(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(LogoutObserver);
  Q_ASSERT(MozillaVPN::instance().userState() == MozillaVPN::UserLoggingOut);

  connect(&MozillaVPN::instance(), &MozillaVPN::userStateChanged, this,
          &LogoutObserver::userStateChanged);
}

LogoutObserver::~LogoutObserver() { MVPN_COUNT_DTOR(LogoutObserver); }

void LogoutObserver::userStateChanged() {
  if (MozillaVPN::instance().userState() == MozillaVPN::UserNotAuthenticated) {
    emit ready();
    deleteLater();
  }
}
