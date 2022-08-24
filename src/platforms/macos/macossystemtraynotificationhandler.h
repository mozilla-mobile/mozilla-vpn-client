/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H
#define MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H

#include "systemtraynotificationhandler.h"
#include "platforms/macos/macosstatusicon.h"

#include <QMenu>
#include <QSystemTrayIcon>

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

 private:
  MacOSStatusIcon* m_macOSStatusIcon = nullptr;
};

#endif  // MACOSSYSTEMTRAYNOTIFICATIONHANDLER_H
