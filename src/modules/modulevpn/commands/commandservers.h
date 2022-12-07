/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDSERVERS_H
#define COMMANDSERVERS_H

#include "command.h"

class CommandServers final : public Command {
 public:
  explicit CommandServers(QObject* parent);
  ~CommandServers();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDSERVERS_H
