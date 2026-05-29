/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <QProcess>
#include <QtGlobal>

#include "../utils/interfaceconfig.h"

class Obfuscator {
 public:
  explicit Obfuscator(const InterfaceConfig& config);
  ~Obfuscator();

  Obfuscator(const Obfuscator&) = delete;
  Obfuscator& operator=(const Obfuscator&) = delete;

  bool start();
  bool isRunning() const { return m_process.state() != QProcess::NotRunning; }
  quint16 parseListeningPort(const QByteArray& line) const;
  QStringList buildArgs(const InterfaceConfig& config);
  QString binaryName() const;
  // Local UDP port the service is listening on
  // The daemon should set up the WireGuard peer to point to this port on
  // localhost
  quint16 localPort() const { return m_localPort; }

 private:
  QProcess m_process;
  quint16 m_localPort = 0;
};

#endif  // OBFUSCATOR_H
