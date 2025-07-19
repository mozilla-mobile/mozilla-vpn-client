/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wgsessionworker.h"

#include "wgsessionmacos.h"

extern "C" {
#include "wireguard_ffi.h"
}

void WgEncryptWorker::run() {
  QByteArray output;
  output.resize(m_packet.size() + WgSessionMacos::WG_PACKET_OVERHEAD);

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(m_packet.constData());
  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto result = wireguard_write(m_session->m_tunnel, ptr, m_packet.size(),
                                outptr, output.size());
  m_session->processResult(result.op, output.first(result.size));
}

void WgDecryptWorker::run() {
  QByteArray output;
  output.resize(m_packet.size());

  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(m_packet.constData());
  uint8_t* outptr = reinterpret_cast<uint8_t*>(output.data());
  auto res = wireguard_read(m_session->m_tunnel, ptr, m_packet.size(), outptr,
                            output.size());
  m_session->processResult(res.op, output.first(res.size));
}
