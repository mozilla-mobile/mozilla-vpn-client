/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"
#include "leakdetector.h"
#include "logger.h"

#include <QWindow>
#include <QNetworkAccessManager>

namespace {
Logger logger(LOG_MAIN, "QmlEngineHolder");
}  // namespace

// static
QmlEngineHolder* QmlEngineHolder::instance() {
  static auto instance = new QmlEngineHolder();
  return instance;
}

QNetworkAccessManager* QmlEngineHolder::networkAccessManager() {
  return m_engine.networkAccessManager();
}

void QmlEngineHolder::clearCacheInternal() {
  QNetworkAccessManager* nam = networkAccessManager();
  Q_ASSERT(nam);

  nam->clearAccessCache();
  nam->clearConnectionCache();
}

QWindow* QmlEngineHolder::window() const {
  QObject* rootObject = m_engine.rootObjects().first();
  return qobject_cast<QWindow*>(rootObject);
}

void QmlEngineHolder::showWindow() {
  QWindow* w = window();
  Q_ASSERT(w);

  w->show();
  w->raise();
  w->requestActivate();
}

void QmlEngineHolder::hideWindow() {
  QWindow* w = window();
  Q_ASSERT(w);

  w->hide();
}
