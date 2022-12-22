/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDACTIVATE_H
#define COMMANDACTIVATE_H

#include "command.h"

class CommandActivate final : public Command {
 public:
  explicit CommandActivate(QObject* parent);
  ~CommandActivate();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDACTIVATE_H
