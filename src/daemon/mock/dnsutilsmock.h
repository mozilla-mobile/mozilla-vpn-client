/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DNSUTILSMOCK_H
#define DNSUTILSMOCK_H

#include <QHostAddress>
#include <QProcess>
#include <QString>

#include "daemon/dnsutils.h"

class DnsUtilsMock final : public DnsUtils {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DnsUtilsMock)

 public:
  explicit DnsUtilsMock(QObject* parent);
  virtual ~DnsUtilsMock();
  bool updateResolvers(const QString& ifname,
                       const QList<QHostAddress>& resolvers) override;
  bool restoreResolvers() override;
};

#endif  // DNSUTILSMACOS_H
