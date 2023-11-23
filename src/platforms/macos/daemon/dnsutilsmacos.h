/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DNSUTILSMACOS_H
#define DNSUTILSMACOS_H

#include <QHostAddress>
#include <QProcess>
#include <QString>

#include "daemon/dnsutils.h"

class DnsUtilsMacos final : public DnsUtils {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DnsUtilsMacos)

 public:
  explicit DnsUtilsMacos(QObject* parent);
  virtual ~DnsUtilsMacos();
  bool updateResolvers(const QString& ifname,
                       const QList<QHostAddress>& resolvers) override;
  bool restoreResolvers() override;

 private slots:
  void dnsManagerStdoutReady();

 private:
  QProcess m_dnsManager;
};

#endif  // DNSUTILSMACOS_H
