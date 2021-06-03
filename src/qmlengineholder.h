/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QMLENGINEHOLDER_H
#define QMLENGINEHOLDER_H

#include "networkmanager.h"

#include <QQmlApplicationEngine>

class QWindow;

class QmlEngineHolder final : public NetworkManager {
  Q_DISABLE_COPY_MOVE(QmlEngineHolder)

 public:
  QmlEngineHolder();
  ~QmlEngineHolder();

  static QmlEngineHolder* instance();

  static bool exists();

  QQmlApplicationEngine* engine() { return &m_engine; }

  QNetworkAccessManager* networkAccessManager() override;

  QWindow* window() const;
  void showWindow();
  void hideWindow();

 protected:
  void clearCacheInternal() override;

 private:
  QQmlApplicationEngine m_engine;
};

#endif  // QMLENGINEHOLDER_H
