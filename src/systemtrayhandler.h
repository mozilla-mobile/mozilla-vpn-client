/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SYSTEMTRAYHANDLER_H
#define SYSTEMTRAYHANDLER_H

#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

class MozillaVPN;
class QAction;

class SystemTrayHandler final : public QSystemTrayIcon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SystemTrayHandler)

 public:
  enum Message {
    None,
    UnsecuredNetwork,
    CaptivePortalBlock,
    CaptivePortalUnblock,
  };

  static SystemTrayHandler* instance();

  explicit SystemTrayHandler(QObject* parent);
  ~SystemTrayHandler();

  void captivePortalBlockNotificationRequired();
  void captivePortalUnblockNotificationRequired();

  void unsecuredNetworkNotification(const QString& networkName);

  void showNotification(const QString& title, const QString& message,
                        int timerMsec);

  void retranslate();

 signals:
  void notificationShown(const QString& title, const QString& message);

  void notificationClicked(Message message);

 public slots:
  void updateIcon(const QString& icon);

  void updateContextMenu();

  void messageClickHandle();

 private:
  void showHideWindow();

  void maybeActivated(QSystemTrayIcon::ActivationReason reason);

 private:
  QMenu m_menu;

  QAction* m_statusLabel = nullptr;
  QAction* m_lastLocationLabel = nullptr;
  QAction* m_disconnectAction = nullptr;
  QAction* m_separator = nullptr;
  QAction* m_preferencesAction = nullptr;
  QAction* m_showHideLabel = nullptr;
  QAction* m_quitAction = nullptr;
  QMenu* m_helpMenu = nullptr;

  Message m_lastMessage = None;
};

#endif  // SYSTEMTRAYHANDLER_H
