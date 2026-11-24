/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef QPROCESSOBFUSCATOR_H
#define QPROCESSOBFUSCATOR_H

#include <QObject>
#include <QProcess>
#include <QtGlobal>

#include "../utils/interfaceconfig.h"
#include "obfuscator.h"

class QProcessObfuscator final : public Obfuscator {
  Q_OBJECT

 public:
  explicit QProcessObfuscator(const QString& path, const InterfaceConfig& cfg,
                              QObject* parent = nullptr);
  ~QProcessObfuscator();

  bool start() override;
  void stop() override;
  bool restart() override;
  bool isRunning() const override;
  quint16 localPort() const override { return m_localPort; }

  qint64 processId() const { return m_process.processId(); }

  QStringList& arguments() { return m_arguments; }

 signals:
  void started(quint16 port);

 private:
  void stderrReady();
  void stdoutReady();
  void processFinished(int code, QProcess::ExitStatus status);

  static QStringList buildArgs(const InterfaceConfig& config);

  // Launch the relay, blocking until it announces its listening port.
  // A non-zero listenPort forces the relay onto that local port.
  bool launch(quint16 listenPort);

  QStringList m_arguments;
  QProcess m_process;
  quint16 m_localPort = 0;
};

#endif  // QPROCESSOBFUSCATOR_H
