/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DNSUTILSWINDOWS_H
#define DNSUTILSWINDOWS_H

#include "daemon/dnsutils.h"

#include <QHostAddress>
#include <QMap>
#include <QString>

class DnsUtilsWindows final : public DnsUtils {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DnsUtilsWindows)

 public:
  explicit DnsUtilsWindows(QObject* parent);
  virtual ~DnsUtilsWindows();
  bool updateResolvers(const QString& ifname,
                       const QList<QHostAddress>& resolvers) override;
  bool restoreResolvers() override;

 private:
  QString m_ifname;
};

#endif  // DNSUTILSWINDOWS_H
