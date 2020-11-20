/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSPINGSENDWORKER_H
#define WINDOWSPINGSENDWORKER_H

#include "pingsendworker.h"

class WindowsPingSendWorker final : public PingSendWorker {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WindowsPingSendWorker)

 public:
  WindowsPingSendWorker();
  ~WindowsPingSendWorker();

 public slots:
  void sendPing(const QString& destination) override;
};

#endif // WINDOWSPINGSENDWORKER_H
