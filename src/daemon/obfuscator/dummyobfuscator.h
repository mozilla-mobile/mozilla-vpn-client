/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYOBFUSCATOR_H
#define DUMMYOBFUSCATOR_H

#include <QtGlobal>

#include "../utils/interfaceconfig.h"
#include "obfuscator.h"

// No-op obfuscator used on WASM
class DummyObfuscator final : public Obfuscator {
 public:
  explicit DummyObfuscator(const InterfaceConfig& config);
  ~DummyObfuscator() override;

  bool start() override;
  quint16 localPort() const override { return m_localPort; }

 private:
  quint16 m_localPort = 0;
};

#endif  // DUMMYOBFUSCATOR_H
