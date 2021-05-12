/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGSENDWORKER_H
#define PINGSENDWORKER_H

#include <QObject>

class PingSendWorker : public QObject {
  Q_OBJECT

 public slots:
  virtual void sendPing(const QString& destination, const QString& source) = 0;

 signals:
  void pingSucceeded();
  void pingFailed();
};

#endif  // PINGSENDWORKER_H
