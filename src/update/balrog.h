/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BALROG_H
#define BALROG_H

#include "updater.h"

class Balrog final : public Updater {
  Q_DISABLE_COPY_MOVE(Balrog)

 public:
  Balrog(QObject* parent);
  ~Balrog();

  void start() override;

 private:
  static QString userAgent();

  bool processData(const QByteArray& data);
};

#endif  // BALROG_H
