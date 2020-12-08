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

  // compare 2 version strings and return:
  // - -1 if the first one is lower than the second one or if the second one is
  // empty.
  // - 0 if they are equal
  // - 1 if the first one is greater than the second one or if the first one is
  // empty.
  static int compareVersions(const QString& a, const QString& b);

 signals:
  // for testing
  void releaseChecked();

 private slots:
  void runInternal();

 private:
  void schedule();

  [[nodiscard]] bool processData(const QByteArray& data);

 private:
  QTimer m_timer;
};

#endif  // RELEASEMONITOR_H
