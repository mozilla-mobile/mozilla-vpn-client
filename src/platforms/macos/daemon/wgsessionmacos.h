/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WGSESSIONMACOS_H
#define WGSESSIONMACOS_H

#include <QHostAddress>
#include <QObject>
#include <QSocketNotifier>
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

  void setTunSocket(qintptr sd);
  void setNetSocket(qintptr sd);
  void setMtu(int mtu);

  static inline constexpr int WG_MTU_OVERHEAD = 80;

 private slots:
  void netReadyRead(QSocketDescriptor sd, QSocketNotifier::Type type);
  void tunReadyRead(QSocketDescriptor sd, QSocketNotifier::Type type);

 private:
  void encrypt(const QByteArray& data);
  void timeout();
  void processResult(int op, const QByteArray& buf);

  void netInput(const QByteArray& packet);
  void netWrite(const QByteArray& packet);
  void tunWrite(const QByteArray& packet, quint32 family);

  QByteArray mhopEncapsulate(const QByteArray& packet);
  void mhopInput(const QByteArray& packet);
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

  int m_tunmtu;
  int m_tunSocket = -1;
  int m_netSocket = -1;
};

#endif  // WGSESSIONMACOS_H