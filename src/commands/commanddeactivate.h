/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDDEACTIVATE_H
#define COMMANDDEACTIVATE_H

#include "command.h"

class CommandDeactivate final : public Command {
 public:
  explicit CommandDeactivate(QObject* parent);
  ~CommandDeactivate();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDDEACTIVATE_H
