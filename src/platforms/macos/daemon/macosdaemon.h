/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSDAEMON_H
#define MACOSDAEMON_H

#include "daemon.h"

#include <QDateTime>

class QLocalSocket;
class QJsonObject;

class MacOSDaemon final : public Daemon {
 public:
  MacOSDaemon();
  ~MacOSDaemon();

  static MacOSDaemon* instance();

  bool activate(const Config& config) override;

  void status(QLocalSocket* socket);
  void logs(QLocalSocket* socket);

 protected:
  bool run(Op op, const Config& config) override;

 private:
  QDateTime m_connectionDate;
};

#endif  // MACOSDAEMON_H
