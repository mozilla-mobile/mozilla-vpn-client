/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDAEMON_H
#define MACOSDAEMON_H

#include "daemon.h"

class MacOSDaemon final : public Daemon {
 public:
  MacOSDaemon();
  ~MacOSDaemon();

  static MacOSDaemon* instance();

  QByteArray getStatus() override;

  void maybeCleanup();

 protected:
  bool run(Op op, const InterfaceConfig& config) override;
};

#endif  // MACOSDAEMON_H
