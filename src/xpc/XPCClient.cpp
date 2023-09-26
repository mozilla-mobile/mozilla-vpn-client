/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcclient.h"

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>

namespace {
constexpr auto C_SERVICENAME = "org.mozilla.firefox.vpn.daemon";
constexpr auto C_DEFAULT_KEY = "daemon";
}  // namespace

void XPCClient::run() {
  m_serverConnection =
      xpc_connection_create_mach_service(C_SERVICENAME, NULL, 0);

  xpc_connection_set_event_handler(m_serverConnection, ^(xpc_object_t event) {
    handleServerEvent(event);
  });
  xpc_connection_resume(m_serverConnection);
  for (;;) {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,
                       std::numeric_limits<double>::max(), TRUE);
  }
}

void XPCClient::send(const QString msg) {
  auto message = xpc_dictionary_create(NULL, NULL, 0);
  auto xpc_string = xpc_string_create(msg.toLocal8Bit().constData());
  xpc_dictionary_set_value(message, C_DEFAULT_KEY, xpc_string);
  xpc_connection_send_message(m_serverConnection, xpc_string);
};

void XPCClient::handleServerEvent(xpc_object_t event) {
  auto maybeError = getXPCError(event);
  if (!maybeError.isNull()) {
    std::cout << "Error:" << maybeError.toLocal8Bit().constData() << std::endl;
    return;
  }
  auto message = getQStringFromXPCDict(event, C_DEFAULT_KEY);

  emit messageReceived(message);
}
