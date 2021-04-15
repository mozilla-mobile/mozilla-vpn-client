/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vpnconnection.h"
#include "constants.h"
#include "logger.h"

// TODO: support windows

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace nlohmann;
using namespace std;

VPNConnection::~VPNConnection() {
  if (connected()) {
    close(m_socket);
  }
}

bool VPNConnection::connect() {
  assert(!connected());

  m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket == -1) {
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

    close(m_socket);
    m_socket = -1;
    return false;
  }

  return true;
}

bool VPNConnection::writeMessage(const json& body) {
  assert(connected());

  string message = body.dump();

  uint32_t length = (uint32_t)message.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  return send(m_socket, rawLength, sizeof(uint32_t), 0) == sizeof(uint32_t) &&
         send(m_socket, message.c_str(), length, 0) == length;
}

bool VPNConnection::readMessage(nlohmann::json& output) {
  assert(connected());

  char rawLength[sizeof(uint32_t)];
  if (recv(m_socket, rawLength, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
    return false;
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  if (!length || length > Constants::MAX_MSG_SIZE) {
    return false;
  }

  char message[length];
  if (recv(m_socket, message, length, 0) != length) {
    return false;
  }

  string m(message, message + sizeof message / sizeof message[0]);
  output = json::parse(m);
  return true;
}
