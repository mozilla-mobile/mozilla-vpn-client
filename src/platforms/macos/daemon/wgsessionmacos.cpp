/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionmacos.h"

#include <QDateTime>
#include <QNetworkDatagram>
#include <QUdpSocket>

#include "interfaceconfig.h"
#include "leakdetector.h"
#include "logger.h"
#include "daemon/wireguardutils.h"

extern "C" {
#include "wireguard_ffi.h"
}

namespace {
Logger logger("WgSessionMacos");
};  // namespace

WgSessionMacos::WgSessionMacos(const InterfaceConfig& config, QObject* parent)
    : QObject(parent), m_config(config) {
  MZ_COUNT_CTOR(WgSessionMacos);
  logger.debug() << "WgSessionMacos created.";

  // Allocate a new tunnel
  m_tunnel = new_tunnel(config.m_privateKey.toUtf8().constData(),
                        config.m_serverPublicKey.toLocal8Bit().constData(),
                        nullptr, WG_KEEPALIVE_PERIOD, 0);
  connect(&m_timer, &QTimer::timeout, this, &WgSessionMacos::timeout);
  m_timer.setSingleShot(false);
  m_timer.start(1000);
}

WgSessionMacos::~WgSessionMacos() {
  MZ_COUNT_DTOR(WgSessionMacos);
  logger.debug() << "WgSessionMacos destroyed.";
  if (m_tunnel) {
    tunnel_free(m_tunnel);
  }
}

void WgSessionMacos::processResult(int op, const QByteArray& buf) {
  switch (op) {
    case WIREGUARD_DONE:
      break;

    case WRITE_TO_NETWORK:
      emit netOutput(buf);
      break;
    
    case WIREGUARD_ERROR:
      logger.warning() << "Wireguard error";
      break;

    case WRITE_TO_TUNNEL_IPV4:
      [[fallthrough]];
    case WRITE_TO_TUNNEL_IPV6:
      emit decrypted(buf);
      break;
  }
}

void WgSessionMacos::timeout() {
  QByteArray buffer;
  buffer.resize(1500);

  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_tick(m_tunnel, bufptr, buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::encrypt(const QByteArray& data) {
  QByteArray buffer;
  buffer.resize(1500);

  const uint8_t* dataptr = reinterpret_cast<const uint8_t*>(data.constData());
  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_write(m_tunnel, dataptr, data.length(), bufptr,
                                buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::netInput(const QByteArray& data) {
  QByteArray buffer;
  buffer.resize(1500);

  const uint8_t* dataptr = reinterpret_cast<const uint8_t*>(data.constData());
  uint8_t* bufptr = reinterpret_cast<uint8_t*>(buffer.data());
  auto result = wireguard_read(m_tunnel, dataptr, data.length(), bufptr,
                               buffer.length());
  processResult(result.op, buffer.first(result.size));
}

void WgSessionMacos::readyRead() {
  QUdpSocket* sock = qobject_cast<QUdpSocket*>(QObject::sender());
  if (!sock) {
    return;
  }

  while (true) {
    QNetworkDatagram dgram = sock->receiveDatagram();
    if (!dgram.isValid()) {
      return;
    }
    netInput(dgram.data()); 
  }
}

qint64 WgSessionMacos::lastHandshake() const {
  qint64 elapsed = wireguard_stats(m_tunnel).time_since_last_handshake;
  if (elapsed < 0) {
    // No Handshake yet.
    return 0;
  }
  return (QDateTime::currentSecsSinceEpoch() - elapsed) * 1000;
}

qint64 WgSessionMacos::rxData() const {
  return wireguard_stats(m_tunnel).rx_bytes;
}

qint64 WgSessionMacos::txData() const {
  return wireguard_stats(m_tunnel).tx_bytes;
}