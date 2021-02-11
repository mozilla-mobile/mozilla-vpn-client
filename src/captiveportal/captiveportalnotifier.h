/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALNOTIFIER_H
#define CAPTIVEPORTALNOTIFIER_H

#include <QObject>
#include <QTimer>

class CaptivePortalNotifier final : public QObject {
  Q_OBJECT

 public:
  CaptivePortalNotifier();
  ~CaptivePortalNotifier();

  void notifyCaptivePortalBlock();
  void notifyCaptivePortalUnblock();

 signals:
  void notificationCaptivePortalBlockCompleted(bool disconnectionRequired);
  void notificationCaptivePortalUnblockCompleted(bool connectionRequired);

 private:
  void messageClicked();
  void notifyTimerExpired();
  void emitSignal(bool userAccepted);

 private:
  enum {
    Unset,
    // The captive portal has been detected because it blocks the VPN.
    Block,
    // We are back online and we need to inform the user about it.
    Unblock,
  } m_type = Unset;

  QTimer m_notifyTimer;
};

#endif  // CAPTIVEPORTALNOTIFIER_H
