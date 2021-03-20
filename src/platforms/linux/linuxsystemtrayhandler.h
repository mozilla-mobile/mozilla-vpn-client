/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXNOTIFICATIONHANDLER_H
#define LINUXNOTIFICATIONHANDLER_H

#include "systemtrayhandler.h"

#include <QObject>

class LinuxSystemTrayHandler final : public SystemTrayHandler {
  Q_DISABLE_COPY_MOVE(LinuxSystemTrayHandler)

 public:
  static LinuxSystemTrayHandler* instance();

  LinuxSystemTrayHandler(QObject* parent);
  ~LinuxSystemTrayHandler();
  void unsecuredNetworkNotification(const QString& networkName);

 protected:
  void showActionNotification(SystemTrayHandler::Message type,
                              const QString& title, const QString& actionText,
                              int timerMsec);
  void showNotificationInternal(SystemTrayHandler::Message type,
                                const QString& title, const QString& message,
                                int timerMsec);

 private:
  void showMessage(QStringList actions, const QString& summary,
                   const QString& body, int timerMsec);
};

#endif  // LINUXNOTIFICATIONHANDLER_H