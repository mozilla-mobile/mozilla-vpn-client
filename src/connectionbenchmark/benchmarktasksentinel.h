/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKSENTINEL_H
#define BENCHMARKTASKSENTINEL_H

#include <QObject>

class BenchmarkTaskSentinel final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskSentinel)

 public:
  BenchmarkTaskSentinel() = default;
  ~BenchmarkTaskSentinel() { emit destroyed(); }

 signals:
  void destroyed();
};

#endif  // BENCHMARKTASKSENTINEL_H
