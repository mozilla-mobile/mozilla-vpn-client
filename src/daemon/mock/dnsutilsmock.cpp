/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dnsutilsmock.h"

#include "logger.h"

namespace {
Logger logger("DnsUtilsMock");
}  // namespace

DnsUtilsMock::DnsUtilsMock(QObject* parent) : DnsUtils(parent) {
  logger.debug() << "DnsUtilsMock created.";
}

DnsUtilsMock::~DnsUtilsMock() {
  logger.debug() << "DnsUtilsMock destroyed.";
}

bool DnsUtilsMock::updateResolvers(const QString& ifname,
                                    const QList<QHostAddress>& resolvers) {
  Q_UNUSED(ifname);
  Q_UNUSED(resolvers);

  // Success!
  return true;
}

bool DnsUtilsMock::restoreResolvers() {
  // Success!
  return true;
}
