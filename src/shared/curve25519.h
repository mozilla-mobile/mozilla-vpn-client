/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CURVE25519_H
#define CURVE25519_H

#include <QByteArray>

#define CURVE25519_KEY_SIZE 32

class Curve25519 final {
 public:
  static QByteArray generatePublicKey(const QByteArray& privateKey);
};

#endif  // CURVE25519_H
