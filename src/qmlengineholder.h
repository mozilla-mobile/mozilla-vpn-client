/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QMLENGINEHOLDER_H
#define QMLENGINEHOLDER_H

#include "networkmanager.h"

class QQmlEngine;
class QWindow;

class QmlEngineHolder final : public NetworkManager {
  Q_DISABLE_COPY_MOVE(QmlEngineHolder)

 public:
  explicit QmlEngineHolder(QQmlEngine* engine);
  ~QmlEngineHolder();

  static QmlEngineHolder* instance();

  static bool exists();

  QQmlEngine* engine() { return m_engine; }

#ifdef UNIT_TEST
  void replaceEngine(QQmlEngine* engine) { m_engine = engine; }
#endif

  QNetworkAccessManager* networkAccessManager() override;

  QWindow* window() const;
  void showWindow();
  void hideWindow();

 protected:
  void clearCacheInternal() override;

 private:
  QQmlEngine* m_engine = nullptr;
};

#endif  // QMLENGINEHOLDER_H
