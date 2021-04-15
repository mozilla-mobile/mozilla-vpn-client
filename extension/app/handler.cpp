/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "handler.h"
#include "constants.h"
#include "logger.h"

#include <iostream>
#include <sys/select.h>

using namespace nlohmann;
using namespace std;

int Handler::run() {
  while (true) {
    // Let's see if we need to connect to the VPN client before reading any
    // message. We don't care about the result of this operation because we can
    // start reading from STDIN and retry the connection later on.
    maybeConnect();

    fd_set rfds;
    int nfds = 1;  // the STDIN

    FD_ZERO(&rfds);
    FD_SET(0, &rfds);

    if (m_vpnConnection.connected()) {
      FD_SET(m_vpnConnection.socket(), &rfds);
      ++nfds;
    }

    int rv = select(nfds, &rfds, NULL, NULL, NULL);
    if (rv == -1) {
      return false;
    }

    if (!rv) {
      continue;
    }

    // Something to read from STDIN
    if (FD_ISSET(0, &rfds)) {
      Logger::log("STDIN message received");

      json message;
      if (!readMessage(message)) {
        Logger::log("Failed to read from STDIN");
        return false;
      }

      // Maybe we are not connected yet. We need to be connected to send the
      // message to the VPN client.
      if (!maybeConnect()) {
        Logger::log("VPN Client not connected");

        if (!writeMessage({{"error", "vpn-client-down"}})) {
          Logger::log("Failed to write to STDOUT");
          return false;
        }

        continue;
      }

      if (!m_vpnConnection.writeMessage(message)) {
        Logger::log("Failed to write to the VPN Client");
        return false;
      }
    }

    // Something to read from the VPN client
    if (m_vpnConnection.connected() &&
        FD_ISSET(m_vpnConnection.socket(), &rfds)) {
      json message;
      if (!m_vpnConnection.readMessage(message)) {
        Logger::log("Failed to read from the VPN Client");
        return false;
      }

      if (!writeMessage(message)) {
        Logger::log("Failed to write to STDOUT");
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
    return false;
  }

  uint32_t length = *reinterpret_cast<uint32_t*>(rawLength);
  if (!length || length > Constants::MAX_MSG_SIZE) {
    return false;
  }

  char message[length];
  if (fread(message, sizeof(char), length, stdin) != length) {
    return false;
  }

  string m(message, message + sizeof message / sizeof message[0]);
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
    return false;
  }

  if (fwrite(message.c_str(), sizeof(char), length, stdout) != length) {
    return false;
  }

  fflush(stdout);
  return true;
}
