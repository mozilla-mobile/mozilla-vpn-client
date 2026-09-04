/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QPROCESSOBFUSCATOR_H
#define QPROCESSOBFUSCATOR_H

#include <QProcess>
#include <QtGlobal>

#include "../utils/interfaceconfig.h"
#include "obfuscator.h"

class QProcessObfuscator final : public Obfuscator {
 public:
  explicit QProcessObfuscator(const InterfaceConfig& config);
  ~QProcessObfuscator() override;

  bool start() override;
  void stop() override;
  bool restart() override;
  bool isRunning() const override;
  quint16 localPort() const override { return m_localPort; }

 private:
  quint16 parseListeningPort(const QByteArray& line) const;
  QStringList buildArgs(const InterfaceConfig& config,
                        quint16 listenPort) const;
  QString binaryName() const;
  // Launch the relay, blocking until it announces its listening port.
  // A non-zero listenPort forces the relay onto that local port.
  bool launch(quint16 listenPort);

  const InterfaceConfig m_config;
  QString m_binaryFile;
  QProcess m_process;
  quint16 m_localPort = 0;
};

#endif  // QPROCESSOBFUSCATOR_H
