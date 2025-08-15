/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XDGNOTIFICATIONHANDLER_H
#define XDGNOTIFICATIONHANDLER_H

#include <QObject>

#include "systemtraynotificationhandler.h"
#include "xdgportal.h"

class XdgNotificationHandler final : public SystemTrayNotificationHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XdgNotificationHandler)

 public:
  explicit XdgNotificationHandler(QObject* parent = nullptr);
  ~XdgNotificationHandler();

 public:
  void initialize() override;
  void notify(Message type, const QString& title, const QString& message,
              int timerMsec) override;

 private slots:
  void notifyFinished(const QDBusMessage& msg);
  void actionInvoked(const QString& id, const QString& action,
                     const QVariantMap& params);

 private:
  QString m_lastTitle;
  QString m_lastBody;
  XdgPortal* m_portal = nullptr;
};

#endif  // XDGNOTIFICATIONHANDLER_H
