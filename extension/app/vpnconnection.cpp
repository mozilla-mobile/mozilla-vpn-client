/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vpnconnection.h"
#include "constants.h"
#include "logger.h"

using namespace nlohmann;
using namespace std;

#ifdef MVPN_WINDOWS
#  define VPN_INVALID_SOCKET INVALID_SOCKET
#else
#  define VPN_INVALID_SOCKET -1
#endif

VPNConnection::VPNConnection() {
#ifdef MVPN_WINDOWS
  // Initialize Winsock
  int rv = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
  if (rv != 0) {
    Logger::log("Failed to initialize the winsocket");
    assert(rv == 0);
  }
#endif
}

VPNConnection::~VPNConnection() {
  if (connected()) {
    closeAndReset();
  }

#ifdef MVPN_WINDOWS
  WSACleanup();
#endif
}

bool VPNConnection::connect() {
  assert(!connected());

  m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket == VPN_INVALID_SOCKET) {
    Logger::log("Failed to open the socket");
    return false;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(Constants::VPN_CLIENT_HOST);
  addr.sin_port = htons(Constants::VPN_CLIENT_PORT);

  if (::connect(m_socket, (sockaddr*)&addr, sizeof(addr)) != 0) {
    Logger::log("Failed to connect to the socket");
    closeAndReset();
    return false;
  }

  return true;
}

bool VPNConnection::writeMessage(const json& body) {
  assert(connected());

  string message = body.dump();

  uint32_t length = (uint32_t)message.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  if (!write(rawLength, sizeof(uint32_t)) || !write(message.c_str(), length)) {
    Logger::log("Failed to write to the VPN Client");
    closeAndReset();
    return false;
  }

  return true;
}

bool VPNConnection::readMessage(nlohmann::json& output) {
  assert(connected());

  char rawLength[sizeof(uint32_t)];
  if (!read(rawLength, sizeof(uint32_t))) {
    Logger::log("Failed to read from the VPN Client");
    closeAndReset();
    return false;
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  if (!length || length > Constants::MAX_MSG_SIZE) {
    Logger::log("Invalid package size from the VPN Client");
    closeAndReset();
    return false;
  }

  char* message = (char*)malloc(length);
  if (!message) {
    Logger::log("Failed to allocate the message buffer");
    closeAndReset();
    return false;
  }

  if (!read(message, length)) {
    Logger::log("Failed to read from the VPN Client");
    closeAndReset();
    free(message);
    return false;
  }

  string m(message, message + sizeof message / sizeof message[0]);
  free(message);

  output = json::parse(m);
  return true;
}

void VPNConnection::closeAndReset() {
  assert(m_socket != -1);
  closesocket(m_socket);
  m_socket = VPN_INVALID_SOCKET;
}

bool VPNConnection::write(const char* buffer, uint32_t length) {
  // The message can be processed in chuncks. Let's do consecutive send() calls
  // until we have sent the full message.

  uint32_t n = 0;
  while (n < length) {
    int rv = send(m_socket, buffer + n, length - n, 0);
    if (rv <= 0) {
      return false;
    }

    n += rv;
  }

  return true;
}

bool VPNConnection::read(char* buffer, uint32_t length) {
  // The message can be received in chuncks. Let's do consecutive recv() calls
  // until we have the full message.
  uint32_t n = 0;
  while (n < length) {
    int rv = recv(m_socket, buffer + n, length - n, 0);
    if (rv <= 0) {
      return false;
    }

    n += rv;
  }

  return true;
}
