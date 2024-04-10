/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsutilsmacos.h"

#include <QCoreApplication>
#include <QProcess>

#include "logger.h"
#include "utils/leakdetector/leakdetector.h"

namespace {
Logger logger("DnsUtilsMacos");
Logger dnsManagerLogger("MacosDnsManager");
}  // namespace

DnsUtilsMacos::DnsUtilsMacos(QObject* parent) : DnsUtils(parent) {
  MZ_COUNT_CTOR(DnsUtilsMacos);

  m_dnsManagerProcess.setProcessChannelMode(QProcess::MergedChannels);
  connect(&m_dnsManagerProcess, SIGNAL(readyReadStandardOutput()), this,
          SLOT(dnsManagerStdoutReady()));

  logger.debug() << "DnsUtilsMacos created.";
}

DnsUtilsMacos::~DnsUtilsMacos() {
  MZ_COUNT_DTOR(DnsUtilsMacos);
  restoreResolvers();
  logger.debug() << "DnsUtilsMacos destroyed.";
}

void DnsUtilsMacos::dnsManagerStdoutReady() {
  for (;;) {
    QByteArray line = m_dnsManagerProcess.readLine();
    if (line.length() <= 0) {
      break;
    }
    dnsManagerLogger.debug() << QString::fromUtf8(line);
  }
}

// Re-launch ourself using the `macosdnsmanager` command to update the system
// DNS configuration. This will run as a separate process so that the DNS
// settings will always be restored no matter how the daemon exits.
bool DnsUtilsMacos::updateResolvers(const QString& ifname,
                                    const QList<QHostAddress>& resolvers) {
  Q_UNUSED(ifname);

  // Prepare the process arguments.
  QStringList args;
  args.append("macosdnsmanager");
  for (const QHostAddress& addr : resolvers) {
    args.append(addr.toString());
  }

  // Launch the DNS manager process.
  m_dnsManagerProcess.start(qApp->applicationFilePath(), args);
  if (!m_dnsManagerProcess.waitForStarted()) {
    m_dnsManagerProcess.kill();
    return false;
  }

  // Success!
  return true;
}

// Restore the DNS configuration by terminating the DNS manager process.
bool DnsUtilsMacos::restoreResolvers() {
  if (m_dnsManagerProcess.state() == QProcess::NotRunning) {
    return true;
  }

  // Terminate the process gracefully with SIGTERM.
  m_dnsManagerProcess.terminate();
  if (!m_dnsManagerProcess.waitForFinished()) {
    m_dnsManagerProcess.kill();
    return false;
  }

  // Success!
  return true;
}
