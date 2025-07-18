/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONMACOS_H
#define WGSESSIONMACOS_H

#include <QHostAddress>
#include <QObject>
#include <QTimer>

#include "interfaceconfig.h"
#include "daemon/wireguardutils.h"

struct wireguard_tunnel;
struct ipv4header;

class WgSessionMacos final : public QObject {
  Q_OBJECT

 public:
  WgSessionMacos(const InterfaceConfig& config, QObject* parent = nullptr);
  ~WgSessionMacos();

  const QString& pubkey() const { return m_config.m_serverPublicKey; }
  WireguardUtils::PeerStatus status() const;

  void renegotiate();
  void encrypt(const QByteArray& data);
  void netInput(const QByteArray& data);

  // Multihop helpers
  QByteArray mhopEncapsulate(const QByteArray& packet);
  void mhopInput(const QByteArray& packet);

 signals:
  void netOutput(const QByteArray& data);
  void mhopOutput(const QByteArray& data);
  void decrypted(const QByteArray& data);

 public slots:
  void readyRead();

 protected:
  void connectNotify(const QMetaMethod& signal) override;

 private:
  void processResult(int op, const QByteArray& buf);
  void timeout();

  void mhopInputV4(const QByteArray& packet);
  void mhopInputV6(const QByteArray& packet);
  void mhopInputUDP(const QHostAddress& src, const QHostAddress& dst,
                    const QByteArray& packet);

  QByteArray mhopDefragV4(const struct ipv4header* header,
                          const QByteArray& packet);

  static quint16 inetChecksum(const void* data, size_t len, quint32 seed = 0);
  static quint16 udpChecksum(const QHostAddress& src, const QHostAddress& dst,
                             quint16 sport, quint16 dport,
                             const QByteArray& payload);

  const InterfaceConfig m_config;

  // Some fields saved for multihop processing.
  bool m_mhopEnabled = false;
  quint16 m_innerPort;
  quint16 m_serverPort;
  quint8 m_innerTTL;
  QHostAddress m_innerIpv4;
  QHostAddress m_innerIpv6;
  QHostAddress m_serverIpv4;
  QHostAddress m_serverIpv6;

  QTimer m_timer;

  // Support for IPv4 defragmentation
  struct Ipv4DefragState {
   public:
    qint64 m_timeout;
    QByteArray m_buffer;
  };
  QHash<quint16, struct Ipv4DefragState> m_defrag;

  // The boringtun instance
  struct wireguard_tunnel* m_tunnel = nullptr;
};

#endif  // WGSESSIONMACOS_H