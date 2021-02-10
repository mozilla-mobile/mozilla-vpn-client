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

  void notify();

 signals:
  void notificationCompleted(bool disconnectionRequired);

 private:
  void messageClicked();
  void notifyTimerExpired();

 private:
  QTimer m_notifyTimer;
};

#endif  // CAPTIVEPORTALNOTIFIER_H
