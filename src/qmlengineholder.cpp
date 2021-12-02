/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"
#include "logger.h"

#include <QWindow>
#include <QNetworkAccessManager>
#include <QApplication>

namespace {
Logger logger(LOG_MAIN, "QmlEngineHolder");
}  // namespace

QmlEngineHolder::QmlEngineHolder() {
  // The engine needs to be cleaned up by Qt.  Do not delete it.
  m_engine = new QQmlApplicationEngine(QApplication::instance());
}

// static
QmlEngineHolder& QmlEngineHolder::instance() {
  static QmlEngineHolder instance;
  return instance;
}

QNetworkAccessManager* QmlEngineHolder::networkAccessManager() {
  return m_engine->networkAccessManager();
}

void QmlEngineHolder::clearCacheInternal() {
  QNetworkAccessManager* nam = networkAccessManager();
  Q_ASSERT(nam);

  nam->clearAccessCache();
  nam->clearConnectionCache();
}

QWindow* QmlEngineHolder::window() const {
  QObject* rootObject = m_engine->rootObjects().first();
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
