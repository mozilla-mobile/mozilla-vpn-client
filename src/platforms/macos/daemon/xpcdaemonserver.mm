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
#include "version.h"

constexpr const int XPC_SESSION_MAX_BACKLOG = 32;

namespace {
Logger logger("XpcDaemonServer");
}  // namespace

// This property exists, but it's private. Make it available:
@interface NSXPCConnection(PrivateAuditToken)
@property (nonatomic, readonly) audit_token_t auditToken;
@end

@interface XpcDaemonDelegate : NSObject<NSXPCListenerDelegate>
@property Daemon* daemon;
@property (copy) NSString* teamIdentifier;

- (id)initWithObject:(Daemon*)daemon;
+ (NSString*)getTeamIdentifier:(SecTaskRef)task;
@end

@interface XpcSessionDelegate : NSObject<XpcDaemonProtocol>
@property Daemon* daemon;
@property XpcDaemonSession* bridge;

- (id)initWithObject:(Daemon*)daemon;
@end

XpcDaemonServer::XpcDaemonServer(Daemon* daemon) : QObject(daemon) {
  MZ_COUNT_CTOR(XpcDaemonServer);

  QString daemonId = MacOSUtils::appId() + ".daemon";
  logger.debug() << "XpcDaemonServer created:" << daemonId;

  XpcDaemonDelegate* delegate =
      [[XpcDaemonDelegate alloc] initWithObject:daemon];
  NSXPCListener* listener =
      [[NSXPCListener alloc] initWithMachServiceName:daemonId.toNSString()];
  listener.delegate = delegate;

  // Connections to the daemon require codesigning
  // TODO: It would be nice if we could turn this off for developers somehow.
  if (@available(macOS 13, *)) {
    constexpr const char* oidExtAppleCodesign =
        "(certificate leaf[field.1.2.840.113635.100.6.1.13] or" \
        " certificate leaf[field.1.2.840.113635.100.6.1.12])";
    constexpr const char* oidExtAppleCaIntermediate =
        "(certificate 1[field.1.2.840.113635.100.6.2.6] or" \
        " certificate 1[field.1.2.840.113635.100.6.2.1])";
    QString devTeamIdentifier =
        QString::fromNSString(delegate.teamIdentifier);
    QString devTeamSubject =
        QString("certificate leaf[subject.OU] = \"%1\"").arg(devTeamIdentifier);

    QStringList xpcCodesignList;
    xpcCodesignList.append("anchor apple generic");
    xpcCodesignList.append(devTeamSubject);
    xpcCodesignList.append(oidExtAppleCodesign);
    xpcCodesignList.append(oidExtAppleCaIntermediate);
    QString xpcCodesignReq = xpcCodesignList.join(" and ");
    [listener setConnectionCodeSigningRequirement:xpcCodesignReq.toNSString()];
  }

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

  SecTaskRef task = SecTaskCreateFromSelf(kCFAllocatorDefault);
  if (task) {
    self.teamIdentifier = [XpcDaemonDelegate getTeamIdentifier:task];
    CFRelease(task);
  }

  return self;
}

+ (NSString*) getTeamIdentifier:(SecTaskRef)task {
  CFErrorRef error = nullptr;
  CFStringRef cfTeamId = CFSTR("com.apple.developer.team-identifier");
  CFTypeRef result = SecTaskCopyValueForEntitlement(task, cfTeamId, &error);
  if (error != nullptr) {
    logger.warning() << "Failed to determine team identifier";
    CFRelease(error);
    return nil;
  }
  auto guard = qScopeGuard([&]() { CFRelease(result); });

  if (CFGetTypeID(result) == CFStringGetTypeID()) {
    return static_cast<NSString*>(result);
  }
  CFRelease(result);
  return nil;
}

- (BOOL)         listener:(NSXPCListener *) listener
shouldAcceptNewConnection:(NSXPCConnection *) newConnection {
  logger.debug() << "new connection";

  if (@available(macOS 13, *)) {
    // Nothing to do here - this is handled by the listener.
  } else {
    // For macOS versions prior to 13.0 we must roll our own connection auth
    // code. This uses the private auditToken property on the NSXPCConnection
    // and inspects the caller's entitlements for a matching team identifier.
    //
    // Sadly there is no public API to get this information, and doing it via
    // the caller's PID is vulnerable to race conditions.
    SecTaskRef task = SecTaskCreateWithAuditToken(kCFAllocatorDefault,
                                                  newConnection.auditToken);
    if (!task) {
      logger.debug() << "rejecting connection: unable to locate calling task";
      return false;
    }
    NSString* clientTeamIdentifier = [XpcDaemonDelegate getTeamIdentifier:task];
    CFRelease(task);
    if ([clientTeamIdentifier compare:self.teamIdentifier] != NSOrderedSame) {
      logger.debug() << "rejecting connection:" << clientTeamIdentifier;
      return false;
    }
  }

  XpcSessionDelegate* delegate = [XpcSessionDelegate alloc];
  delegate.bridge = new XpcDaemonSession(self.daemon, newConnection);

  newConnection.exportedObject = [delegate initWithObject:self.daemon];
  newConnection.exportedInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcDaemonProtocol)];
  newConnection.remoteObjectInterface =
      [NSXPCInterface interfaceWithProtocol:@protocol(XpcClientProtocol)];
  newConnection.invalidationHandler = ^{
    logger.debug() << "closed connection";
    delegate.bridge->deleteLater();
  };

  [newConnection resume];
  return true;
}
@end

@implementation XpcSessionDelegate
- (id)initWithObject:(Daemon*)daemon {
  self = [super init];
  self.daemon = daemon;
  return self;
}

- (void)activate:(NSString*) config {
  QMetaObject::invokeMethod(self.daemon, "activate", QString::fromNSString(config));
}

- (void)deactivate {
  QMetaObject::invokeMethod(self.daemon, "deactivate");
}

- (void)getVersion: (void (^)(NSString *))reply {
  reply([NSString stringWithUTF8String:APP_VERSION]);
}

- (void)getStatus:(void (^)(NSString *))reply {
  NSString* result = nullptr;
  NSCondition* cond = [NSCondition new];

  [cond lock];
  QMetaObject::invokeMethod(self.daemon, [&]{
    QByteArray jsBlob = QJsonDocument(self.daemon->getStatus()).toJson();
    result = [NSString stringWithUTF8String:jsBlob.constData()];
    [cond signal];
  });

  [cond wait];
  reply(result);
}

- (void) getBackendLogs: (void (^)(NSString *))reply {
  NSString* result = nullptr;
  NSCondition* cond = [NSCondition new];

  [cond lock];
  QMetaObject::invokeMethod(self.daemon, [&](){
    result = self.daemon->logs().toNSString();
    [cond signal];
  });

  [cond wait];
  reply(result);
}

- (void)cleanupBackendLogs {
  QMetaObject::invokeMethod(self.daemon, [&]{ self.daemon->cleanLogs(); });
}

@end

XpcDaemonSession::XpcDaemonSession(Daemon* daemon, void* connection)
    : QObject(nullptr), m_connection(connection) {
  connect(daemon, &Daemon::connected, this, &XpcDaemonSession::connected);
  connect(daemon, &Daemon::disconnected, this, &XpcDaemonSession::disconnected);
  connect(daemon, &Daemon::backendFailure, this,
          &XpcDaemonSession::backendFailure);

  // Move this object to the same thread as the daemon, and make it our parent.
  // The XPC connection runs in its own thread, so it lacks a Qt event loop to
  // process signals.
  moveToThread(daemon->thread());
  setParent(daemon);
}

// It's not entirely clear if the ObjC runtime will handle thread-safety when
// invoking methods on the remoteObjectProxy for us. So this wrapper will take
// a method to be invoked and schedule it for execution on the XPC thread with
// scheduleSendBarrierBlock.
//
// This also counts the number of method calls waiting on the connection to
// detect if the client has stopped processing messages, in which case we might
// need to abandon the connection in order to prevent memory leaks.
void XpcDaemonSession::invokeClient(SEL selector) {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  if (m_backlog.fetchAndAddOrdered(1) > XPC_SESSION_MAX_BACKLOG) {
    [conn invalidate];
    return;
  }

  [[conn remoteObjectProxy] performSelector:selector];
  [conn scheduleSendBarrierBlock:^{ m_backlog.fetchAndSubOrdered(1); }];
}

// Same as above - but it can take an argument.
template <typename T>
void XpcDaemonSession::invokeClient(SEL selector, T arg) {
  NSXPCConnection* conn = static_cast<NSXPCConnection*>(m_connection);
  if (m_backlog.fetchAndAddOrdered(1) > XPC_SESSION_MAX_BACKLOG) {
    [conn invalidate];
    return;
  }

  NSObject* remote = [conn remoteObjectProxy];
  NSMethodSignature* sig = [remote methodSignatureForSelector:selector];
  NSInvocation* invocation = [NSInvocation invocationWithMethodSignature:sig];
  [invocation setSelector:selector];
  [invocation setArgument:&arg atIndex:2];
  [invocation invokeWithTarget:remote];
  [conn scheduleSendBarrierBlock:^{ m_backlog.fetchAndSubOrdered(1); }];
}

void XpcDaemonSession::connected(const QString& pubkey) {
  invokeClient(@selector(connected:), pubkey.toNSString());
}

void XpcDaemonSession::disconnected() {
  invokeClient(@selector(disconnected));
}

void XpcDaemonSession::backendFailure(DaemonError reason) {
  invokeClient(@selector(backendFailure:), static_cast<NSUInteger>(reason));
}
