/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDSTATUS_H
#define COMMANDSTATUS_H

#include "command.h"

class CommandStatus final : public Command {
 public:
  explicit CommandStatus(QObject* parent);
  ~CommandStatus();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDSTATUS_H
