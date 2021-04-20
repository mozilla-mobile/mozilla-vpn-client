/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants {
constexpr uint32_t MAX_MSG_SIZE = 1024 * 1024;

constexpr uint32_t VPN_CLIENT_PORT = 8754;
constexpr const char* VPN_CLIENT_HOST = "127.0.0.1";
}  // namespace Constants

#endif  // CONSTANTS_H
