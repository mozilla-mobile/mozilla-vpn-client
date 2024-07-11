/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NOTIFICATIONHANDLER_H
#define NOTIFICATIONHANDLER_H

#include <QObject>

class Addon;
class QMenu;

#ifdef UNIT_TEST
class TestAddon;
#endif

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
    NewInAppMessage,
    SubscriptionNotFound,
    ConnectionFailure,
  };

  static NotificationHandler* create(QObject* parent);

  static NotificationHandler* instance();

  virtual ~NotificationHandler();

  void captivePortalBlockNotificationRequired();
  void captivePortalUnblockNotificationRequired();

  void unsecuredNetworkNotification(const QString& networkName);

  void serverUnavailableNotification(bool pingRecieved);

  void newInAppMessageNotification(const QString& title,
                                   const QString& message);

  void subscriptionNotFoundNotification();

  void connectionFailureNotification();

  void showNotification();

  void messageClickHandle();

  virtual void retranslate() {}

#ifdef MZ_WASM
  virtual QMenu* contextMenu() { return nullptr; }
#endif

 signals:
  void notificationShown(const QString& title, const QString& message);

  void notificationClicked(NotificationHandler::Message message);

 protected:
  explicit NotificationHandler(QObject* parent);

  virtual void notify(Message type, const QString& title,
                      const QString& message, int timerMsec) = 0;

  virtual void initialize() {}

 private:
  static NotificationHandler* createInternal(QObject* parent);

  virtual void notifyInternal(Message type, const QString& title,
                              const QString& message, int timerMsec);

  void addonCreated(Addon* addon);
  void maybeAddonNotification(Addon* addon);

 protected:
  Message m_lastMessage = None;

 private:
  bool m_switching = false;

  // We want to show a 'disconnected' notification only if we were actually
  // connected.
  bool m_connected = false;

#ifdef UNIT_TEST
  friend class TestAddon;
#endif
};

#endif  // NOTIFICATIONHANDLER_H
