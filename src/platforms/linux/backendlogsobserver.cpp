/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "backendlogsobserver.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFile>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("BackendLogsObserver");
}

BackendLogsObserver::BackendLogsObserver(
    QObject* parent, std::function<void(const QString&)>&& callback)
    : QObject(parent), m_callback(std::move(callback)) {
  MZ_COUNT_CTOR(BackendLogsObserver);
}

BackendLogsObserver::~BackendLogsObserver() {
  MZ_COUNT_DTOR(BackendLogsObserver);
}

void BackendLogsObserver::completed(QDBusPendingCallWatcher* call) {
  QDBusPendingReply<QString> reply = *call;
  
  if (!reply.isError()) {
    QString status = reply.argumentAt<0>();
    m_callback(status);
  } else {
    logger.error() << "Error received from service:" << reply.error().message();

    // Otherwise, try our best to scrape the logs directly off disk.
    QFile logfile("/var/log/mozillavpn.log");
    if (!logfile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
      m_callback(QString("Failed to open backend logs"));
    } else {
      m_callback(logfile.readAll());
    }
  }
}
