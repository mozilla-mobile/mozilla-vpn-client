/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDGRAPHICGUESS_H
#define COMMANDGRAPHICGUESS_H

#include "command.h"

class CommandGraphicGuess final : public Command {
 public:
  explicit CommandGraphicGuess(QObject* parent);
  ~CommandGraphicGuess();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDGRAPHICGUESS_H
