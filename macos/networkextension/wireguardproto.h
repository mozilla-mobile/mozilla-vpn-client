/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WIREGUARDPROTO_H

#include <stdint.h>

constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_INIT = 0x01;
constexpr const uint8_t WG_MSG_TYPE_HANDSHAKE_RESP = 0x02;
constexpr const uint8_t WG_MSG_TYPE_COOKIE_REPLY = 0x03;
constexpr const uint8_t WG_MSG_TYPE_DATA = 0x04;

constexpr const size_t WG_MSG_LEN_MIN = 4;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_INIT = 148;
constexpr const size_t WG_MSG_LEN_HANDSHAKE_RESP = 92;
constexpr const size_t WG_MSG_LEN_COOKIE_REPLY = 64;
constexpr const size_t WG_MSG_LEN_DATA_HEADER = 16;

#endif  // WIREGUARDPROTO_H
