/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXBYPASS_H
#define LINUXBYPASS_H

#include <QObject>

class Socks5;
class QAbstractSocket;
class QHostAddress;

class LinuxBypass final : public QObject {
  Q_OBJECT

 public:
  explicit LinuxBypass(Socks5* proxy);
  ~LinuxBypass() = default;

 private slots:
  void outgoingConnection(QAbstractSocket* s, const QHostAddress& dest);
};

#endif  // LINUXBYPASS_H
