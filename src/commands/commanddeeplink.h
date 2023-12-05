/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDDEEPLINK_H
#define COMMANDDEEPLINK_H

#include "command.h"

class CommandDeepLink final : public Command {
 public:
  explicit CommandDeepLink(QObject* parent);
  ~CommandDeepLink();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDDEEPLINK_H
