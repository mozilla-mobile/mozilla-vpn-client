/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcservice.h"

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>

#include <QDebug>
#include <QObject>
#include <QtLogging>

#include "leakdetector.h"

namespace {

}  // namespace

XPCService::XPCService(QString aServiceName, QString aSigningRequirement)
    : QObject(),
      mServiceName(aServiceName),
      mSigningRequirement(aSigningRequirement) {
  MZ_COUNT_CTOR(XPCService);
};

XPCService::~XPCService() { MZ_COUNT_DTOR(XPCService); }

void XPCService::start() {
  qDebug() << "[XPC] - Starting server: " << qUtf8Printable(mServiceName);
  // Create the new xpc_service

  // TODO: it seems launchctl also writes the env variable XPC_SERVICE_NAME
  // for us, that's super convinent!
  xpc_connection_t listener = xpc_connection_create_mach_service(
      qUtf8Printable(mServiceName), NULL,
      XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
  if (Q_UNLIKELY(listener == NULL)) {
    qDebug() << "[XPC] - Failed to create listener: ";
    return;
  }
  // event listener
  xpc_connection_set_event_handler(listener, ^(xpc_object_t xpc_event) {
    // New connections arrive here.
    xpc_type_t type = xpc_get_type(xpc_event);
    if (type == XPC_TYPE_ERROR) {
      auto error = getXPCError(xpc_event);
      qCritical() << "[XPC Error] " << qUtf8Printable(error);
      return;
    }
    if (type != XPC_TYPE_CONNECTION) {
      // This listener should not get anything but connections?
      qWarning() << "Unexpected type: " << xpc_type_get_name(type);
      Q_ASSERT(false);
      return;
    }
    // Increase the refcount by 1 so we may keep a ref.
    // We release this obj on close or stop
    auto xpc_peer = (xpc_connection_t)xpc_retain(xpc_event);

    maybeEnforceSigningRequirement(xpc_peer);
    acceptConnectionRequest(xpc_peer);
  });
  // Activate the connection
  xpc_connection_activate(listener);
  m_listener = listener;
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
  qDebug() << "Accepted XPC-Connection";
}

void XPCService::handleClientEvent(xpc_object_t event,
                                   xpc_connection_t client) {
  auto guard = qScopeGuard([&] { xpc_release(event); });
  xpc_type_t type = xpc_get_type(event);
  if (type == XPC_TYPE_ERROR) {
    auto error = getXPCError(event);
    qCritical() << "[XPC Error] " << qUtf8Printable(error);
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
  // Make sure that we only allow mozilla signed connections.
  // if a connection does not match the requirement's all
  // messages it sends will be dismissed by the OS.
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
