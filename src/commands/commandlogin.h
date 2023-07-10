/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef COMMANDLOGIN_H
#define COMMANDLOGIN_H

#include "command.h"

class CommandLogin final : public Command {
 public:
  explicit CommandLogin(QObject* parent);
  ~CommandLogin();

  int run(QStringList& tokens) override;

 private:
  static QString getInput(const QString& prompt);
  static QString getPassword(const QString& prompt);
};

#endif  // COMMANDLOGIN_H
