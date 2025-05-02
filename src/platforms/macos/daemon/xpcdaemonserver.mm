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

@interface XpcDaemonDelegate : NSObject<NSXPCListenerDelegate>
@property Daemon* daemon;
- (id)initWithObject:(Daemon*)daemon;
@end

@interface XpcSessionDelegate : NSObject<XpcDaemonProtocol>
@property Daemon* daemon;
@property XpcDaemonSession* bridge;

- (id)initWithObject:(Daemon*)daemon
       andConnection:(NSXPCConnection*)conn;
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

  XpcSessionDelegate* delegate = [XpcSessionDelegate alloc];
  newConnection.exportedObject = [delegate initWithObject:self.daemon
                                            andConnection:newConnection];
  newConnection.exportedInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  newConnection.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];
  newConnection.invalidationHandler = ^{
    logger.debug() << "closed connection";
  };

  [newConnection resume];
  return true;
}
@end

@implementation XpcSessionDelegate
- (id)initWithObject:(Daemon*)daemon
       andConnection:(NSXPCConnection*)conn {
  self = [super init];
  self.daemon = daemon;
  self.bridge = new XpcDaemonSession(daemon, conn);
  return self;
}

- (void)dealloc {
  self.bridge->deleteLater();
  [super dealloc];
}

- (void)activate:(NSString*) config {
  QMetaObject::invokeMethod(self.daemon, "activate", QString::fromNSString(config));
}

- (void)deactivate {
  QMetaObject::invokeMethod(self.daemon, "deactivate");
}

- (void)getStatus {
  QMetaObject::invokeMethod(self.daemon, [self]{
    QJsonObject obj = self.daemon->getStatus();
    QString js = QString(QJsonDocument(obj).toJson());
    self.bridge->invokeClient(@selector(status:), js);
  });
}

@end

XpcDaemonSession::XpcDaemonSession(Daemon* daemon, void* connection)
    : QObject(nullptr), m_connection(connection) {
  connect(daemon, &Daemon::connected, this, &XpcDaemonSession::connected);
  connect(daemon, &Daemon::disconnected, this, &XpcDaemonSession::disconnected);

  // Move this object to the same thread as the daemon, and make it our parent.
  // The XPC connection runs in its own thread, so it lacks a Qt event loop to
  // process signals.
  moveToThread(daemon->thread());
  setParent(daemon);
}

// It's not entirely clear if the ObjC runtime will handle thread-safety when
// invoking methods on the remoteObjectProxy for us. So this wrapper will take
// a method to be invoked (with an optional argument) and schedule it for
// execution on the XPC thread with scheduleSendBarrierBlock.
//
// This also counts the number of method calls waiting on the connection to
// detect if the client has stopped processing messages, in which case we might
// need to abandon the connection in order to prevent memory leaks.
void XpcDaemonSession::invokeClient(SEL selector, const QString& arg) {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  if (m_backlog.fetchAndAddOrdered(1) > 32) {
    [conn invalidate];
    return;
  }

  if (arg.isNull()) {
    [conn scheduleSendBarrierBlock:^{
      [[conn remoteObjectProxy] performSelector: selector];
      m_backlog.fetchAndSubOrdered(1);
    }];
  } else {
    NSString* nsArg = arg.toNSString();
    [conn scheduleSendBarrierBlock:^{
      [[conn remoteObjectProxy] performSelector: selector
                                     withObject: nsArg];
      m_backlog.fetchAndSubOrdered(1);
    }];
  }
}

void XpcDaemonSession::connected(const QString& pubkey) {
  invokeClient(@selector(connected:), pubkey);
}

void XpcDaemonSession::disconnected() {
  invokeClient(@selector(disconnected));
}
