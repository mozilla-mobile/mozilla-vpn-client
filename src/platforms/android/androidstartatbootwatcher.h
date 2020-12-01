/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDSTARTATBOOTWATCHER_H
#define ANDROIDSTARTATBOOTWATCHER_H

#include <QObject>

class AndroidStartAtBootWatcher final : public QObject {
 public:
  AndroidStartAtBootWatcher(bool startAtBoot);

 public slots:
  void startAtBootChanged(bool value);
};

#endif  // ANDROIDSTARTATBOOTWATCHER_H
