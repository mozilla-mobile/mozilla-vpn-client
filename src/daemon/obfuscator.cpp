/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "obfuscator.h"

#include <QByteArray>
#include <cstdint>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("Obfuscator");
}

Obfuscator::Obfuscator(const InterfaceConfig& config) {
  MZ_COUNT_CTOR(Obfuscator);
  // QByteArrays own the UTF-8 buffers for the duration of the FFI call
  // the Rust side copies what it needs before returning.
  QByteArray ipv4Utf8 = config.m_serverIpv4AddrIn.toUtf8();
  QByteArray ipv6Utf8 = config.m_serverIpv6AddrIn.toUtf8();
  QByteArray serverPubKeyUtf8 = config.m_serverPublicKey.toUtf8();
  QByteArray publicKeyUtf8 = config.m_publicKey.toUtf8();

  ObfuscatorConfig cfg{};
  cfg.obfuscation_method = static_cast<uint32_t>(config.m_obfuscationMethod);
  cfg.server_ipv4_addr_in = ipv4Utf8.constData();
  cfg.server_ipv6_addr_in = ipv6Utf8.constData();
  cfg.server_port = static_cast<uint16_t>(config.m_serverPort);
  cfg.server_public_key = serverPubKeyUtf8.constData();
  cfg.public_key = publicKeyUtf8.constData();

  logger.debug() << "Starting obfuscator" << config.m_obfuscationMethod;

  m_handle = obfuscator_start(&cfg);
  if (!m_handle) {
    logger.error() << "Failed to start obfuscator"
                   << config.m_obfuscationMethod;
    return;
  }
  m_localPort = obfuscator_local_port(m_handle);
  m_socketV4 = obfuscator_socket_v4(m_handle);
  m_socketV6 = obfuscator_socket_v6(m_handle);
  logger.debug() << "Obfuscator" << config.m_obfuscationMethod
                 << "listening on 127.0.0.1:" << m_localPort;
}

Obfuscator::~Obfuscator() {
  MZ_COUNT_DTOR(Obfuscator);
  if (m_handle) {
    logger.debug() << "Stopping obfuscator";
    obfuscator_stop(m_handle);
    m_handle = nullptr;
  }
}
