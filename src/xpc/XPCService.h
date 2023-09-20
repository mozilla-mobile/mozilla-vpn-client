/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef XPCSERVICE_H
#define XPCSERVICE_H

#include <stdio.h>
#import <xpc/xpc.h>
#import <xpc/xpcservice.h>
#include <QList>
#include <QObject>
#include <QScopeGuard>
#include <QThread>
#include <iostream>

class XPCService : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(XPCService)

 public:
  explicit XPCService() : QObject(){};
  ~XPCService() = default;

  void run() {
    // Create the new xpc_service
    xpc_connection_t listener = xpc_connection_create_mach_service(
        "org.mozilla.firefox.vpn.daemon", NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);
    // event listener
    xpc_connection_set_event_handler(listener, ^(xpc_object_t xpc_event) {
      // New connections arrive here.
      // Make sure that we only allow mozilla signed connections
      // if a connection does not match the requirement's all
      // messages it sends will be dismissed by the OS.
      auto xpc_conection = (xpc_connection_t)xpc_event;
      if (__builtin_available(macOS 12.0, *)) {
        // TODO: Find out how to handle.
        // The result is an int, but nowhere are mappings, what each values mean q_q
        int res = xpc_connection_set_peer_code_signing_requirement(xpc_conection, "org.mozilla");
        Q_UNUSED(res);
      }

      std::cout << "New XPC-Connection" << std::endl;
      aceptConnectionRequest(xpc_conection);
    });
    // Activate the connection
    xpc_connection_resume(listener);
    for (;;) {
      // Run the Apple Event Loop
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, DBL_MAX, TRUE);
    }
  }

  void aceptConnectionRequest(xpc_connection_t client) {
    // Install an event handler for that connection
    xpc_connection_set_event_handler(client, ^(xpc_object_t event) {
      handleClientEvent(event, client);
    });
    // Keep a ref internally
    m_clients.append(client);
    // This start's the listening for client messages.
    xpc_connection_resume(client);
  }
  void closeClientConnection(xpc_connection_t client) {
    xpc_connection_cancel(client);
    if (m_clients.contains(client)) {
      m_clients.removeAll(client);
    }
    xpc_release(client);
  }

  void handleClientEvent(xpc_object_t event, xpc_connection_t client) {
    auto guard = qScopeGuard([&] { xpc_release(event); });

    xpc_type_t type = xpc_get_type(event);
    if (type == XPC_TYPE_ERROR) {
      if (event == XPC_ERROR_CONNECTION_INVALID) {
        // The client process on the other end of the connection has either
        // crashed or cancelled the connection.
        std::cout << "xpc_peer_connection invalid" << std::endl;
      }
      if (event == XPC_ERROR_TERMINATION_IMMINENT) {
        std::cout << "xpc_peer_connection terminated" << std::endl;
      }
      if (__builtin_available(macOS 12.0, *)) {
        if (event == XPC_ERROR_PEER_CODE_SIGNING_REQUIREMENT) {
          std::cout << "XPC_ERROR_PEER_CODE_SIGNING_REQUIREMENT failed" << std::endl;
        }
      }
      closeClientConnection(client);
      return;
    }

    if (type != XPC_TYPE_STRING) {
      // We only send JSON Strings
      // so someone is breaking contract.
      closeClientConnection(client);
      return;
    }
    assert(type == XPC_TYPE_STRING);
    size_t len = xpc_string_get_length(event);
    if (len == 0) {
      return;
    }
    const char* str_ptr = xpc_string_get_string_ptr(event);
    if (str_ptr == nullptr) {
      return;
    }
    // This will copy the string into a new QString.
    QString owned_string = QString::fromLocal8Bit(str_ptr, len);
    std::cout << "Received a Message" << std::endl;
    emit messageRecieved(owned_string);
  };

  void send(const QString msg) {
    auto message = xpc_dictionary_create(NULL, NULL, 0);
    auto xpc_string = xpc_string_create(msg.toLocal8Bit().constData());
    xpc_dictionary_set_value(message, "vpnMessage", xpc_string);
    for (const auto& client : m_clients) {
      xpc_connection_send_message(client, xpc_string);
    }
  };

 signals:
  void messageRecieved(const QString msg);

 private:
  QList<xpc_connection_t> m_clients;
};

#endif  // XPCSERVICE_H
