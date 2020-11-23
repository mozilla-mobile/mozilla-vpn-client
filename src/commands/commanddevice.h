/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDDEVICE_H
#define COMMANDDEVICE_H

#include "command.h"

class CommandDevice final : public Command {
 public:
  CommandDevice(QObject* parent);
  ~CommandDevice();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDDEVICE_H
