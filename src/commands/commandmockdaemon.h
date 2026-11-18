/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDMOCKDAEMON_H
#define COMMANDMOCKDAEMON_H

#include "command.h"

class CommandMockDaemon final : public Command {
 public:
  explicit CommandMockDaemon(QObject* parent);
  ~CommandMockDaemon();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDMOCKDAEMON_H
