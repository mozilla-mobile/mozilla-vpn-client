/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEMTRAYNOTIFICATIONHANDLER_H
#define SYSTEMTRAYNOTIFICATIONHANDLER_H

#include "notificationhandler.h"

#include <QSystemTrayIcon>

class QAction;
class QMenu;

class SystemTrayNotificationHandler : public NotificationHandler {
 public:
  explicit SystemTrayNotificationHandler(QObject* parent);
  ~SystemTrayNotificationHandler();

  void retranslate() override;

#ifdef MVPN_WASM
  QMenu* contextMenu() override;
#endif

 protected:
  virtual void notify(Message type, const QString& title,
                      const QString& message, int timerMsec) override;

  virtual void showHideWindow();

  virtual void setStatusMenu();

  virtual void updateIcon();

 protected:
  QScopedPointer<QMenu> m_menu;
  QSystemTrayIcon* m_systemTrayIcon;

 private:
  void createStatusMenu();

  void maybeActivated(QSystemTrayIcon::ActivationReason reason);

  void updateContextMenu();

 private:
  QAction* m_statusLabel = nullptr;
  QAction* m_lastLocationLabel = nullptr;
  QAction* m_disconnectAction = nullptr;
  QAction* m_separator = nullptr;
  QAction* m_preferencesAction = nullptr;
  QAction* m_showHideLabel = nullptr;
  QAction* m_quitAction = nullptr;
  QAction* m_helpAction = nullptr;
};

#endif  // SYSTEMTRAYNOTIFICATIONHANDLER_H
