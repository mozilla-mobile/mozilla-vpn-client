/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CAPTIVEPORTALMONITOR_H
#define CAPTIVEPORTALMONITOR_H

#include <QObject>
#include <QTimer>

class CaptivePortalMonitor final : public QObject {
  Q_OBJECT

 public:
  CaptivePortalMonitor();
  ~CaptivePortalMonitor();

  void start();
  void stop();

 signals:
  void online();

  private:
  void check();

 private:
  QTimer m_timer;
};

#endif  // CAPTIVEPORTALMONITOR_H
