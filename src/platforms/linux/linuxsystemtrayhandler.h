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
  void captivePortalBlockNotificationRequired();
  void captivePortalUnblockNotificationRequired();
  void unsecuredNetworkNotification(const QString& networkName);

 private:
  void showUnityActionNotification(SystemTrayHandler::Message type,
                                   const QString& title,
                                   const QString& actionText, int timerMsec);
  bool m_isUnity = true;
};

#endif  // LINUXNOTIFICATIONHANDLER_H