/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef VPNCONNECTION_H
#define VPNCONNECTION_H

#include "json.hpp"

#ifdef MVPN_WINDOWS
#  include <winsock2.h>
#  include <windows.h>
#endif

class VPNConnection final {
 public:
  VPNConnection();
  ~VPNConnection();

  bool connected() const {
#ifdef MVPN_WINDOWS
    return m_socket != INVALID_SOCKET;
#else
    return m_socket != -1;
#endif
  }

#ifdef MVPN_WINDOWS
  const SOCKET&
#else
  int
#endif
  socket() const {
    return m_socket;
  }

  bool connect();

  bool readMessage(nlohmann::json& output);
  bool writeMessage(const nlohmann::json& body);

 private:
  bool write(const char* buffer, uint32_t length);
  bool read(char* buffer, uint32_t length);

  void closeAndReset();

 private:
#ifdef MVPN_WINDOWS
  SOCKET m_socket = INVALID_SOCKET;

  WSADATA m_wsaData;
#else
  int m_socket = -1;
#endif
};

#endif  // VPNCONNECTION_H
