/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qmlengineholder.h"

#include <QApplication>
#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QQmlNetworkAccessManagerFactory>
#include <QWindow>

#include "leakdetector.h"
#include "networkmanager.h"

#ifdef MZ_WINDOWS
#  include <windows.h>
#endif

namespace {
QmlEngineHolder* s_instance = nullptr;

class NMFactory : public QQmlNetworkAccessManagerFactory, public QObject {
 public:
  NMFactory(QObject* parent = nullptr)
      : QQmlNetworkAccessManagerFactory(), QObject(parent) {}
  QNetworkAccessManager* create(QObject* parent) override {
    return NetworkManager::instance()->networkAccessManager();
  }
};

}  // namespace

QmlEngineHolder::QmlEngineHolder(QQmlEngine* engine) : m_engine(engine) {
  MZ_COUNT_CTOR(QmlEngineHolder);

  Q_ASSERT(engine);
  Q_ASSERT(!s_instance);
  s_instance = this;
  engine->setNetworkAccessManagerFactory(new NMFactory(qApp));
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
  if (!w) {
    return;
  }

  w->show();

  w->requestActivate();
#ifdef MZ_WINDOWS
  auto const windowHandle = (HWND)w->winId();
  SetWindowPos(windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  SetForegroundWindow(windowHandle);
  SetWindowPos(windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0,
               SWP_NOMOVE | SWP_NOSIZE);
#else
  w->raise();
#endif
}

void QmlEngineHolder::hideWindow() {
  QWindow* w = window();
  Q_ASSERT(w);

  w->hide();
}
