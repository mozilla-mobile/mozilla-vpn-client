/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXNOTIFICATIONHANDLER_H
#define LINUXNOTIFICATIONHANDLER_H

#include "systemtrayhandler.h"

#include <QObject>

class LinuxSystemTrayHandler final : public SystemTrayHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(LinuxSystemTrayHandler)

 public:
  static bool requiredCustomImpl();

  LinuxSystemTrayHandler(QObject* parent);
  ~LinuxSystemTrayHandler();

 private:
  void showNotificationInternal(Message type, const QString& title,
                                const QString& message, int timerMsec) override;

 private slots:
  void actionInvoked(uint actionId, QString action);

 private:
  uint m_lastNotificationId = 0;
};

#endif  // LINUXNOTIFICATIONHANDLER_H
