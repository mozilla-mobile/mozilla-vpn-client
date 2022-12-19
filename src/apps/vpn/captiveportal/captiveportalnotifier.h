/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALNOTIFIER_H
#define CAPTIVEPORTALNOTIFIER_H

#include <QTimer>

#include "notificationhandler.h"

class CaptivePortalNotifier final : public QObject {
  Q_OBJECT

 public:
  explicit CaptivePortalNotifier(QObject* parent);
  ~CaptivePortalNotifier();

  void notifyCaptivePortalBlock();
  void notifyCaptivePortalUnblock();

 signals:
  void activationRequired();

 private:
  void notificationClicked(NotificationHandler::Message message);
};

#endif  // CAPTIVEPORTALNOTIFIER_H
