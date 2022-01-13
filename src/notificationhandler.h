/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include <QObject>

class QMenu;

class NotificationHandler : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NotificationHandler)

 public:
  enum Message {
    None,
    UnsecuredNetwork,
    CaptivePortalBlock,
    CaptivePortalUnblock,
    ServerUnavailable,
  };

  static NotificationHandler* create(QObject* parent);

  static NotificationHandler* instance();

  virtual ~NotificationHandler();

  void captivePortalBlockNotificationRequired();
  void captivePortalUnblockNotificationRequired();

  void unsecuredNetworkNotification(const QString& networkName);

  void serverUnavailableNotification();

  void showNotification();

  void messageClickHandle();

  virtual void retranslate() {}

#ifdef MVPN_WASM
  virtual QMenu* contextMenu() { return nullptr; }
#endif

 signals:
  void notificationShown(const QString& title, const QString& message);

  void notificationClicked(Message message);

 protected:
  explicit NotificationHandler(QObject* parent);

  virtual void notify(Message type, const QString& title,
                      const QString& message, int timerMsec) = 0;

 private:
  virtual void notifyInternal(Message type, const QString& title,
                              const QString& message, int timerMsec);

 protected:
  Message m_lastMessage = None;

 private:
  QString m_switchingLocalizedServerCountry;
  QString m_switchingLocalizedServerCity;
  bool m_switching = false;

  // We want to show a 'disconnected' notification only if we were actually
  // connected.
  bool m_connected = false;
};

#endif  // NOTIFICATIONHANDLER_H
