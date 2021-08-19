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
  struct peerStatus {
    QString pubkey;
    qint64 handshake;
    qint64 rxBytes;
    qint64 txBytes;
  };

  explicit WireguardUtils(QObject* parent) : QObject(parent){};
  virtual ~WireguardUtils() = default;

  virtual bool interfaceExists() = 0;
  virtual QString interfaceName() { return WG_INTERFACE; }
  virtual bool addInterface(const InterfaceConfig& config) = 0;
  virtual bool deleteInterface() = 0;

  virtual bool updatePeer(const InterfaceConfig& config) = 0;
  virtual bool deletePeer(const QString& pubkey) = 0;
  virtual QList<peerStatus> getPeerStatus() = 0;

  virtual bool updateRoutePrefix(const IPAddressRange& prefix,
                                 int hopindex) = 0;
  virtual bool deleteRoutePrefix(const IPAddressRange& prefix,
                                 int hopindex) = 0;
};

#endif  // WIREGUARDUTILS_H
