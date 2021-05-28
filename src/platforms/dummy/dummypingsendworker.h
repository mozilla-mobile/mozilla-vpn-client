/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYPINGSENDWORKER_H
#define DUMMYPINGSENDWORKER_H

#include "pingsendworker.h"

class DummyPingSendWorker final : public PingSendWorker {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DummyPingSendWorker)

 public:
  DummyPingSendWorker();
  ~DummyPingSendWorker();

 public slots:
  void sendPing(const QString& destination, const QString& source) override;
};

#endif  // DUMMYPINGSENDWORKER_H
