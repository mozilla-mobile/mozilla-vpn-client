/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "logoutobserver.h"
#include "core.h"
#include "leakdetector.h"

LogoutObserver::LogoutObserver(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(LogoutObserver);

  Core* core = Core::instance();
  Q_ASSERT(core->userAuthenticated());

  connect(core, &Core::userAuthenticationChanged, this,
          &LogoutObserver::userAuthenticationChanged);
}

LogoutObserver::~LogoutObserver() { MVPN_COUNT_DTOR(LogoutObserver); }

void LogoutObserver::userAuthenticationChanged() {
  Core* core = Core::instance();
  if (!core->userAuthenticated()) {
    emit ready();
    deleteLater();
  }
}
