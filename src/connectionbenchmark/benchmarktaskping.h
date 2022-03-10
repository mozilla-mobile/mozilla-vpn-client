/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BENCHMARKTASKPING_H
#define BENCHMARKTASKPING_H

#include "benchmarktask.h"
#include "networkrequest.h"

#include <QObject>

class BenchmarkTaskPing : public BenchmarkTask {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(BenchmarkTaskPing)

 public:
  BenchmarkTaskPing();
  ~BenchmarkTaskPing();

 signals:
  void finished(quint64 pingLatency);

 private:
  void handleState(BenchmarkTask::State state);

 private:
  double m_pingLatencyAcc = 0;
  quint8 m_numOfPingSamples = 0;
};

#endif  // BENCHMARKTASKPING_H
