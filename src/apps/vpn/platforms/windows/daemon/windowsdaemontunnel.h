/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSDAEMONTUNNEL_H
#define WINDOWSDAEMONTUNNEL_H

#include "command.h"

class WindowsDaemonTunnel final : public Command {
 public:
  explicit WindowsDaemonTunnel(QObject* parent);
  ~WindowsDaemonTunnel();

  int run(QStringList& tokens) override;
};

#endif  // WINDOWSDAEMONTUNNEL_H
