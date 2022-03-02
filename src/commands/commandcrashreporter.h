/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDCRASHREPORTER_H
#define COMMANDCRASHREPORTER_H

#include "command.h"

class CommandCrashReporter final : public Command {
 public:
  explicit CommandCrashReporter(QObject* parent);
  ~CommandCrashReporter();

  int run(QStringList& tokens) override;
};

#endif  // COMMANDCRASHREPORTER_H
