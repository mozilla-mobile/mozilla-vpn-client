/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosdaemon.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocalSocket>
#include <QProcess>
#include <QSettings>
#include <QTextStream>
#include <QtGlobal>

#include "dnsutilsmacos.h"
#include "firewallutilsmacos.h"
#include "iputilsmacos.h"
#include "leakdetector.h"
#include "logger.h"
#include "wireguardutilsmacos.h"

namespace {
Logger logger("MacOSDaemon");
}  // namespace

MacOSDaemon::MacOSDaemon(QObject* parent) : Daemon(parent) {
  MZ_COUNT_CTOR(MacOSDaemon);
  logger.debug() << "Daemon created";

  m_wgutils = new WireguardUtilsMacos(this);
  m_dnsutils = new DnsUtilsMacos(this);
  m_iputils = new IPUtilsMacos(this);
  m_fwutils = new FirewallUtilsMacos(this);

  connect(m_wgutils, &WireguardUtils::backendFailure, this,
          &MacOSDaemon::abortBackendFailure);
}

MacOSDaemon::~MacOSDaemon() {
  MZ_COUNT_DTOR(MacOSDaemon);
  logger.debug() << "Daemon released";
}
