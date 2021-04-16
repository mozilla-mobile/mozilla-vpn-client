/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HANDLER_H
#define HANDLER_H

#include "vpnconnection.h"
#include "json.hpp"

class Handler final {
 public:
  int run();

 private:
  static bool readMessage(nlohmann::json& output);
  static bool writeMessage(const nlohmann::json& body);

  bool maybeConnect();

  bool writeVpnNotConnected();

 private:
  VPNConnection m_vpnConnection;
};

#endif  // HANDLER_H
