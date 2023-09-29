/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcservice.h"

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>

#include "leakdetector.h"

namespace {

}  // namespace

XPCService::XPCService(QString aServiceName, QString aSigningRequirement)
    : QObject(),
      mServiceName(aServiceName),
      mSigningRequirement(aSigningRequirement) {
  MZ_COUNT_CTOR(XPCService);
  connect(this, &XPCService::runAppleEventLoop, this,
          &XPCService::onRunAppleEventLoop, Qt::QueuedConnection);
};

XPCService::~XPCService() { MZ_COUNT_DTOR(XPCService); }

void XPCService::start() {
  // Create the new xpc_service
  xpc_connection_t listener = xpc_connection_create_mach_service(
      qUtf8Printable(mServiceName), NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);

  // event listener
  xpc_connection_set_event_handler(listener, ^(xpc_object_t xpc_event) {
    // New connections arrive here.
    // Make sure that we only allow mozilla signed connections
    // if a connection does not match the requirement's all
    // messages it sends will be dismissed by the OS.
    std::cout << "New XPC-Connection" << std::endl;
    auto xpc_peer = (xpc_connection_t)xpc_event;
    maybeEnforceSigningRequirement(xpc_peer);
    acceptConnectionRequest(xpc_peer);
  });
  // Activate the connection
  xpc_connection_resume(listener);
  m_listener = listener;

  emit runAppleEventLoop();
}
void XPCService::stop() {
  for (const auto& client : m_clients) {
    closeClientConnection(client);
  }
  xpc_connection_cancel(m_listener);
  xpc_release(m_listener);
  m_listener = NULL;
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
  auto message = getQStringFromXPCDict(event, defaultDictKey);
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
  xpc_dictionary_set_value(message, defaultDictKey, xpc_string);
  for (const auto& client : m_clients) {
    xpc_connection_send_message(client, xpc_string);
  }
};

void XPCService::maybeEnforceSigningRequirement(xpc_connection_t peer) {
  if (mSigningRequirement.isEmpty()) {
    return;
  }
  if (__builtin_available(macOS 12.0, *)) {
    // TODO: Find out how to handle.
    // The result is an int, but nowhere are mappings, what each values mean
    // q_q
    int res = xpc_connection_set_peer_code_signing_requirement(
        peer, qUtf8Printable(mSigningRequirement));
    Q_UNUSED(res);
  }
};

void XPCService::onRunAppleEventLoop() {
  // auto h = std::hash<std::thread::id>{}(std::this_thread::get_id());
  //  qWarning() << "[XPCClient::onRunAppleEventLoop] - Thread " <<
  //  qUtf8Printable(QString::number(h));

  CFRunLoopRunInMode(kCFRunLoopDefaultMode,
                     0.5,  // Run the apple event loop max 500ms
                     TRUE  // Return after 1 Handled event
  );
  // Emit the next loop run onto the
  // Qt event loop.
  emit runAppleEventLoop();
}
