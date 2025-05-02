/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xpcdaemonserver.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "leakdetector.h"
#include "logger.h"
#include "platforms/macos/macosutils.h"
#include "platforms/macos/xpcdaemonprotocol.h"

namespace {
Logger logger("XpcDaemonServer");
}  // namespace

@interface XpcDaemonDelegate : NSObject<NSXPCListenerDelegate, XpcDaemonProtocol>

@property Daemon* daemon;
- (id)initWithObject:(Daemon*)daemon;

- (void) activate:(NSString*) config;
- (void) deactivate;

@end

XpcDaemonServer::XpcDaemonServer(Daemon* daemon) : QObject(daemon) {
  MZ_COUNT_CTOR(XpcDaemonServer);

  QString daemonId = MacOSUtils::appId() + ".daemon";
  logger.debug() << "XpcDaemonServer created:" << daemonId;

  XpcDaemonDelegate* delegate = [XpcDaemonDelegate alloc];

  NSXPCListener* listener =
      [[NSXPCListener alloc] initWithMachServiceName:daemonId.toNSString()];
  listener.delegate = [delegate initWithObject:daemon];
  [listener retain];
  [listener resume];

  m_listener = listener;
}

XpcDaemonServer::~XpcDaemonServer() {
  MZ_COUNT_DTOR(XpcDaemonServer);
  NSXPCListener* listener = static_cast<NSXPCListener*>(m_listener);
  logger.debug() << "XpcDaemonServer destroyed.";
  [listener release];
}

@implementation XpcDaemonDelegate
- (id)initWithObject:(Daemon*)daemon {
  self = [super init];
  self.daemon = daemon;
  return self;
}

- (BOOL)         listener:(NSXPCListener *) listener
shouldAcceptNewConnection:(NSXPCConnection *) newConnection {
  logger.debug() << "new connection";

  newConnection.exportedObject = listener.delegate;
  newConnection.exportedInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  newConnection.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];

  // Create a connection bridge for the reverse direction.
  auto bridge = new XpcSessionBridge(self.daemon, newConnection);
  newConnection.invalidationHandler = ^{
    logger.debug() << "closed connection";
    bridge->deleteLater();
  };

  [newConnection resume];
  return true;
}

- (void)activate:(NSString*) config {
  QMetaObject::invokeMethod(self.daemon, "activate", QString::fromNSString(config));
}

- (void)deactivate {
  QMetaObject::invokeMethod(self.daemon, "deactivate");
}

@end

XpcSessionBridge::XpcSessionBridge(Daemon* daemon, void* connection)
    : QObject(nullptr), m_connection(connection) {
  connect(daemon, &Daemon::connected, this, &XpcSessionBridge::connected);
  connect(daemon, &Daemon::disconnected, this, &XpcSessionBridge::disconnected);

  // Move this object to the same thread as the daemon, and make it our parent.
  // The XPC connection runs in its own thread, so it lacks a Qt event loop to
  // process signals.
  moveToThread(daemon->thread());
  setParent(daemon);
}

void XpcSessionBridge::connected(const QString& pubkey) {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  [[conn remoteObjectProxy] connected:pubkey.toNSString()];
}

void XpcSessionBridge::disconnected() {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  [[conn remoteObjectProxy] disconnected];
}
