/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcservice.h"

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>

#include "leakdetector.h"

namespace {
constexpr auto C_SERVICENAME = "org.mozilla.firefox.vpn.daemon";
constexpr auto C_DEFAULT_KEY = "daemon";
}  // namespace

XPCService::XPCService() : QThread() { MZ_COUNT_CTOR(XPCService); };

XPCService::~XPCService() { MZ_COUNT_DTOR(XPCService); }

void XPCService::run() {
  // Create the new xpc_service
  xpc_connection_t listener = xpc_connection_create_mach_service(
      C_SERVICENAME, NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);
  // event listener
  xpc_connection_set_event_handler(listener, ^(xpc_object_t xpc_event) {
    // New connections arrive here.
    // Make sure that we only allow mozilla signed connections
    // if a connection does not match the requirement's all
    // messages it sends will be dismissed by the OS.
    auto xpc_conection = (xpc_connection_t)xpc_event;
    if (__builtin_available(macOS 12.0, *)) {
      // TODO: Find out how to handle.
      // The result is an int, but nowhere are mappings, what each values mean
      // q_q
      int res = xpc_connection_set_peer_code_signing_requirement(xpc_conection,
                                                                 "org.mozilla");
      Q_UNUSED(res);
    }

    std::cout << "New XPC-Connection" << std::endl;
    acceptConnectionRequest(xpc_conection);
  });
  // Activate the connection
  xpc_connection_resume(listener);
  for (;;) {
    // Run the Apple Event Loop
    CFRunLoopRunInMode(kCFRunLoopDefaultMode,
                       std::numeric_limits<double>::max(), TRUE);
  }
}

void XPCService::acceptConnectionRequest(xpc_connection_t client) {
  // Install an event handler for that connection
  xpc_connection_set_event_handler(client, ^(xpc_object_t event) {
    handleClientEvent(event, client);
  });
  // Keep a ref internally
  m_clients.append(client);
  // This start's the listening for client messages.
  xpc_connection_resume(client);
  std::cout << "Accepted XPC-Connection" << std::endl;
}

void XPCService::handleClientEvent(xpc_object_t event,
                                   xpc_connection_t client) {
  auto guard = qScopeGuard([&] { xpc_release(event); });
  xpc_type_t type = xpc_get_type(event);
  if (type == XPC_TYPE_ERROR) {
    auto error = getXPCError(event);
    std::cout << error.toLocal8Bit().constData();
    closeClientConnection(client);
    return;
  }
  if (type != XPC_TYPE_DICTIONARY) {
    // ALL XPC messages are dictionaries.
    // If it's not, something has invoked this function wrong
    closeClientConnection(client);
    return;
  }
  auto message = getQStringFromXPCDict(event, C_DEFAULT_KEY);
  std::cout << "Received a Message" << std::endl;
  emit messageReceived(message);
};

void XPCService::closeClientConnection(xpc_connection_t client) {
  xpc_connection_cancel(client);
  if (m_clients.contains(client)) {
    m_clients.removeAll(client);
  }
  xpc_release(client);
}

void XPCService::send(const QString msg) {
  auto message = xpc_dictionary_create(NULL, NULL, 0);
  auto xpc_string = xpc_string_create(msg.toLocal8Bit().constData());
  xpc_dictionary_set_value(message, C_DEFAULT_KEY, xpc_string);
  for (const auto& client : m_clients) {
    xpc_connection_send_message(client, xpc_string);
  }
};
