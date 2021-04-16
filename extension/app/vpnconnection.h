/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCONNECTION_H
#define VPNCONNECTION_H

#include "json.hpp"

class VPNConnection final {
 public:
  ~VPNConnection();

  bool connected() const { return m_socket != -1; }

  int socket() const { return m_socket; }

  bool connect();

  bool readMessage(nlohmann::json& output);
  bool writeMessage(const nlohmann::json& body);

 private:
  void closeAndReset();

 private:
  int m_socket = -1;
};

#endif  // VPNCONNECTION_H
