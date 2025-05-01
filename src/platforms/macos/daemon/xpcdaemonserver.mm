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
- (id)initWithObject:(Daemon*)controller;

- (BOOL)         listener:(NSXPCListener *) listener
shouldAcceptNewConnection:(NSXPCConnection *) newConnection;

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
  logger.debug() << "XpcDaemonServer new connection";
  newConnection.exportedObject = listener.delegate;
  newConnection.exportedInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  newConnection.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];
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
