/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDUI_H
#define COMMANDUI_H

#include "command.h"

class CommandUI final : public Command {
 public:
  explicit CommandUI(QObject* parent);
  ~CommandUI();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDUI_H
