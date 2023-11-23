/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsutilsmacos.h"

#include <QCoreApplication>
#include <QProcess>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("DnsUtilsMacos");
}

DnsUtilsMacos::DnsUtilsMacos(QObject* parent) : DnsUtils(parent) {
  MZ_COUNT_CTOR(DnsUtilsMacos);

  m_dnsManager.setProcessChannelMode(QProcess::MergedChannels);
  connect(&m_dnsManager, SIGNAL(readyReadStandardOutput()), this,
          SLOT(dnsManagerStdoutReady()));

  logger.debug() << "DnsUtilsMacos created.";
}

DnsUtilsMacos::~DnsUtilsMacos() {
  MZ_COUNT_DTOR(DnsUtilsMacos);
  if (!restoreResolvers()) {
    m_dnsManager.kill();
  }
  logger.debug() << "DnsUtilsMacos destroyed.";
}

void DnsUtilsMacos::dnsManagerStdoutReady() {
  for (;;) {
    QByteArray line = m_dnsManager.readLine();
    if (line.length() <= 0) {
      break;
    }
    logger.debug() << QString::fromUtf8(line);
  }
}

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
  m_dnsManager.start(qApp->applicationFilePath(), args);
  return m_dnsManager.waitForStarted();
}

bool DnsUtilsMacos::restoreResolvers() {
  if (m_dnsManager.state() == QProcess::NotRunning) {
    return true;
  }

  m_dnsManager.terminate();
  return m_dnsManager.waitForFinished();
}
