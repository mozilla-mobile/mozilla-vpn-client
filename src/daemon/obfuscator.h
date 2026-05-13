/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <QtGlobal>

#include "../utils/interfaceconfig.h"
#include "obfuscators.h"

class Obfuscator {
 public:
  explicit Obfuscator(const InterfaceConfig& config);
  ~Obfuscator();

  Obfuscator(const Obfuscator&) = delete;
  Obfuscator& operator=(const Obfuscator&) = delete;

  bool isRunning() const { return m_handle != nullptr; }

  // Local UDP port the service is listening on
  // The daemon should set up the WireGuard peer to point to this port on
  // localhost
  quint16 localPort() const { return m_localPort; }

  int socketV4() const { return m_socketV4; }
  int socketV6() const { return m_socketV6; }

 private:
  ::ObfuscatorHandle* m_handle = nullptr;
  quint16 m_localPort = 0;
  int m_socketV4 = -1;
  int m_socketV6 = -1;
};

#endif  // OBFUSCATOR_H
