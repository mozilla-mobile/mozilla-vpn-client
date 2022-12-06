/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef UPDATEREQUIREDWATCHER_H
#define UPDATEREQUIREDWATCHER_H

#include <QObject>

class UpdateRequiredWatcher final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(UpdateRequiredWatcher)

 public:
  explicit UpdateRequiredWatcher(QObject* parent);
  ~UpdateRequiredWatcher();

  void run();

 private:
  void maybeReadyToUpdate();

 signals:
  void readyToUpdate();

 private:
  int m_count = 0;
};

#endif  // UPDATEREQUIREDWATCHER_H
