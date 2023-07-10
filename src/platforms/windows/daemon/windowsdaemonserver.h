/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSDAEMONSERVER_H
#define WINDOWSDAEMONSERVER_H

#include "command.h"

class WindowsDaemonServer final : public Command {
  Q_DISABLE_COPY_MOVE(WindowsDaemonServer)

 public:
  explicit WindowsDaemonServer(QObject* parent);
  ~WindowsDaemonServer();

  int run(QStringList& tokens) override;
};

#endif  // WINDOWSDAEMONSERVER_H
