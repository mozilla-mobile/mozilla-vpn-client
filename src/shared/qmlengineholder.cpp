/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"

#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QWindow>

#include "leakdetector.h"

namespace {
QmlEngineHolder* s_instance = nullptr;
}  // namespace

QmlEngineHolder::QmlEngineHolder(QQmlEngine* engine) : m_engine(engine) {
  MZ_COUNT_CTOR(QmlEngineHolder);

  Q_ASSERT(engine);
  Q_ASSERT(!s_instance);
  s_instance = this;
}

QmlEngineHolder::~QmlEngineHolder() {
  MZ_COUNT_DTOR(QmlEngineHolder);

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
  return m_engine->networkAccessManager();
}

void QmlEngineHolder::clearCacheInternal() {
  QNetworkAccessManager* nam = networkAccessManager();
  Q_ASSERT(nam);

  nam->clearAccessCache();
  nam->clearConnectionCache();
}

QWindow* QmlEngineHolder::window() const {
  QQmlApplicationEngine* engine =
      qobject_cast<QQmlApplicationEngine*>(m_engine);
  if (!engine) return nullptr;

  Q_ASSERT(hasWindow());
  QObject* rootObject = engine->rootObjects().first();
  return qobject_cast<QWindow*>(rootObject);
}

bool QmlEngineHolder::hasWindow() const {
  return !qobject_cast<QQmlApplicationEngine*>(m_engine)
              ->rootObjects()
              .isEmpty();
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
