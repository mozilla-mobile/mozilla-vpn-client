/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "handler.h"
#include "constants.h"
#include "logger.h"

#include <iostream>

#ifndef MVPN_WINDOWS
#  include <sys/select.h>
#endif

using namespace nlohmann;
using namespace std;

int Handler::run() {
  while (true) {
    // Let's see if we need to connect to the VPN client before reading any
    // message. We don't care about the result of this operation because we can
    // start reading from STDIN and retry the connection later on.
    maybeConnect();

    bool readStdin = false;
    bool readVpnConnection = false;

#ifdef MVPN_WINDOWS
    HANDLE handles[3];
    handles[0] = GetStdHandle(STD_INPUT_HANDLE);
    handles[1] = INVALID_HANDLE_VALUE;
    handles[2] = INVALID_HANDLE_VALUE;

    int count = 1;

    if (m_vpnConnection.connected()) {
      handles[1] = WSACreateEvent();
      WSAEventSelect(m_vpnConnection.socket(), handles[1], FD_READ);
      ++count;
    }

    DWORD rv = WaitForMultipleObjectsEx(count, handles, FALSE, INFINITE, FALSE);

    readStdin = (rv == WAIT_OBJECT_0);
    readVpnConnection = m_vpnConnection.connected() && rv == WAIT_OBJECT_0 + 1;
#else  // POSIX
    fd_set rfds;
    int nfds = 0;  // the STDIN

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    if (m_vpnConnection.connected()) {
      FD_SET(m_vpnConnection.socket(), &rfds);
      nfds = m_vpnConnection.socket();
    }

    int rv = select(nfds + 1, &rfds, NULL, NULL, NULL);
    if (rv == -1) {
      return false;
    }

    if (!rv) {
      continue;
    }

    readStdin = FD_ISSET(0, &rfds);
    readVpnConnection = m_vpnConnection.connected() &&
                        FD_ISSET(m_vpnConnection.socket(), &rfds);
#endif

    // Something to read from STDIN
    if (readStdin) {
      Logger::log("STDIN message received");

      json message;
      if (!readMessage(message)) {
        return false;
      }

      // Maybe we are not connected yet. We need to be connected to send the
      // message to the VPN client.
      if (!maybeConnect()) {
        Logger::log("VPN Client not connected");

        if (!writeVpnNotConnected()) {
          return false;
        }

        continue;
      }

      // The VPN can be terminated at any time. Let's treat it as a non-fatal
      // error.
      if (!m_vpnConnection.writeMessage(message)) {
        assert(!m_vpnConnection.connected());
        if (!writeVpnNotConnected()) {
          return false;
        }

        continue;
      }
    }

    // Something to read from the VPN client
    if (m_vpnConnection.connected() && readVpnConnection) {
      json message;
      if (!m_vpnConnection.readMessage(message)) {
        assert(!m_vpnConnection.connected());
        continue;
      }

      if (!writeMessage(message)) {
        return false;
      }
    }
  }
}

bool Handler::maybeConnect() {
  if (m_vpnConnection.connected()) {
    return true;
  }

  return m_vpnConnection.connect();
}

// Retrieve a message from the STDIN.
// static
bool Handler::readMessage(json& output) {
  char rawLength[sizeof(uint32_t)];
  if (fread(rawLength, sizeof(char), sizeof(uint32_t), stdin) !=
      sizeof(uint32_t)) {
    Logger::log("Failed to read from STDIN");
    return false;
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  if (!length || length > Constants::MAX_MSG_SIZE) {
    Logger::log("Failed to read from STDIN");
    return false;
  }

  char* message = (char*)malloc(length);
  if (!message) {
    Logger::log("Failed to allocate the message buffer");
    return false;
  }

  if (fread(message, sizeof(char), length, stdin) != length) {
    Logger::log("Failed to read from STDIN");
    free(message);
    return false;
  }

  string m(message, message + length);
  free(message);

  output = json::parse(m);
  return true;
}

// Serialize a message to STDOUT
// static
bool Handler::writeMessage(const json& body) {
  string message = body.dump();

  uint32_t length = (uint32_t)message.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  if (fwrite(rawLength, sizeof(char), sizeof(uint32_t), stdout) !=
      sizeof(uint32_t)) {
    Logger::log("Failed to write to STDOUT");
    return false;
  }

  if (fwrite(message.c_str(), sizeof(char), length, stdout) != length) {
    Logger::log("Failed to write to STDOUT");
    return false;
  }

  fflush(stdout);
  return true;
}

bool Handler::writeVpnNotConnected() {
  return writeMessage({{"error", "vpn-client-down"}});
}
