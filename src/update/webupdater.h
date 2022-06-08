/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBUPDATER_H
#define WEBUPDATER_H

#include "updater.h"

class WebUpdater final : public Updater {
  Q_DISABLE_COPY_MOVE(WebUpdater)

 public:
  WebUpdater(QObject* parent);
  ~WebUpdater();

  void start(Task* task) override;
};

#endif  // WEBUPDATER_H
