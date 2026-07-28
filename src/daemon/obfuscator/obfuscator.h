/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H

#include <QtGlobal>

// Generic obfuscator interface.
class Obfuscator {
 public:
  virtual ~Obfuscator() = default;

  Obfuscator(const Obfuscator&) = delete;
  Obfuscator& operator=(const Obfuscator&) = delete;

  virtual bool start() = 0;

  virtual quint16 localPort() const = 0;

 protected:
  Obfuscator() = default;
};

#endif  // OBFUSCATOR_H
