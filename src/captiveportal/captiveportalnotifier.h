/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALNOTIFIER_H
#define CAPTIVEPORTALNOTIFIER_H

#include "systemtrayhandler.h"

#include <QTimer>

class CaptivePortalNotifier final : public QObject {
  Q_OBJECT

 public:
  explicit CaptivePortalNotifier(QObject* parent);
  ~CaptivePortalNotifier();

  void notifyCaptivePortalBlock();
  void notifyCaptivePortalUnblock();

 signals:
  void activationRequired();
  void deactivationRequired();

 private:
  void notificationClicked(SystemTrayHandler::Message message);
};

#endif  // CAPTIVEPORTALNOTIFIER_H
