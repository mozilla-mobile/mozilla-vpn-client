/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDUTILS_H
#define WIREGUARDUTILS_H

#include "interfaceconfig.h"

#include <QObject>
#include <QStringList>
#include <QCoreApplication>

constexpr const char* WG_INTERFACE = "moz0";

class WireguardUtils : public QObject {
  Q_OBJECT

 public:
  struct peerBytes {
    double txBytes, rxBytes;
  };

  explicit WireguardUtils(QObject* parent) : QObject(parent){};
  virtual ~WireguardUtils() = default;

  virtual bool interfaceExists(const QString& ifname) = 0;
  virtual bool addInterface(const InterfaceConfig& config) = 0;
  virtual bool updateInterface(const InterfaceConfig& config) = 0;
  virtual bool deleteInterface(const QString& ifname) = 0;
  virtual peerBytes getThroughputForInterface(const QString& ifname) = 0;
  virtual bool addRoutePrefix(const IPAddressRange& prefix,
                              const QString& ifname) = 0;
  virtual void flushRoutes(const QString& ifname) = 0;
};

#endif  // WIREGUARDUTILS_H
