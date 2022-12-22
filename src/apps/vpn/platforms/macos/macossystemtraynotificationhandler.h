/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H
#define MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H

#include <QMenu>
#include <QSystemTrayIcon>

#include "platforms/macos/macosstatusicon.h"
#include "systemtraynotificationhandler.h"

class MacosSystemTrayNotificationHandler
    : public SystemTrayNotificationHandler {
 public:
  explicit MacosSystemTrayNotificationHandler(QObject* parent);
  ~MacosSystemTrayNotificationHandler();

  void updateIconIndicator();

 protected:
  virtual void showHideWindow() override;

  virtual void setStatusMenu() override;

  virtual void updateIcon() override;

  virtual void notify(Message type, const QString& title,
                      const QString& message, int timerMsec) override;

 private:
  void initialize() override;

 private:
  MacOSStatusIcon* m_macOSStatusIcon = nullptr;
};

#endif  // MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H
