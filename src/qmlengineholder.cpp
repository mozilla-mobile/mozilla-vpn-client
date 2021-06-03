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
QmlEngineHolder* s_instance = nullptr;
}  // namespace

QmlEngineHolder::QmlEngineHolder() {
  MVPN_COUNT_CTOR(QmlEngineHolder);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

QmlEngineHolder::~QmlEngineHolder() {
  MVPN_COUNT_DTOR(QmlEngineHolder);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
QmlEngineHolder* QmlEngineHolder::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
bool QmlEngineHolder::exists() { return !!s_instance; }

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
