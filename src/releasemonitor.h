/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RELEASEMONITOR_H
#define RELEASEMONITOR_H

#include <QObject>
#include <QTimer>

class ReleaseMonitor final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ReleaseMonitor)

 public:
  ReleaseMonitor();
  ~ReleaseMonitor();

  void runSoon();
  void updateSoon();

 signals:
  // for testing
  void releaseChecked();

 private:
  void schedule();

  void updateRequired();
  void updateRecommended();

 private:
  QTimer m_timer;
};

#endif  // RELEASEMONITOR_H
