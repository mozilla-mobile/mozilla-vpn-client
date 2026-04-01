/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WEBEXTCOMMAND_H
#define WEBEXTCOMMAND_H

#include "command.h"

class WebExtCommand final : public Command {
 public:
  explicit WebExtCommand(QObject* parent);
  ~WebExtCommand();

  int run(QStringList& tokens) override;
};

#endif  // WEBEXTCOMMAND_H
