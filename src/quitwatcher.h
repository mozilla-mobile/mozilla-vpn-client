/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QUITWATCHER_H
#define QUITWATCHER_H

#include <QObject>

class QuitWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(QuitWatcher)

 public:
  explicit QuitWatcher(QObject* parent);
  ~QuitWatcher();

 private:
  void maybeReadyToQuit();

 signals:
  void readyToQuit();

 private:
  int m_count = 0;
};

#endif  // QUITWATCHER_H
