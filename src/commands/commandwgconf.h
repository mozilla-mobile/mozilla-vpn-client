/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDWGCONF_H
#define COMMANDWGCONF_H

#include "command.h"

class CommandWgConf final : public Command {
 public:
  explicit CommandWgConf(QObject* parent);
  ~CommandWgConf();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDWGCONF_H
