/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDGPUCHECK_H
#define COMMANDGPUCHECK_H

#include "command.h"

class CommandGpuCheck final : public Command {
 public:
  explicit CommandGpuCheck(QObject* parent);
  ~CommandGpuCheck();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDGPUCHECK_H
