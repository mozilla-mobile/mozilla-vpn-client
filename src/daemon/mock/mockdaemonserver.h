/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOCKDAEMONSERVER_H
#define MOCKDAEMONSERVER_H

#include "command.h"

class MockDaemonServer final : public Command {
 public:
  explicit MockDaemonServer(QObject* parent);
  ~MockDaemonServer();

  int run(QStringList& tokens) override;
};

#endif  // MOCKDAEMONSERVER_H
