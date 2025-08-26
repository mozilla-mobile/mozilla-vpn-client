/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgportal.h"

#include <unistd.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcessEnvironment>
#include <QRandomGenerator>

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
#  include <private/qdesktopunixservices_p.h>
#  include <private/qguiapplication_p.h>
#  include <qpa/qplatformintegration.h>
#elif QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#  include <private/qgenericunixservices_p.h>
#  include <private/qguiapplication_p.h>
#  include <qpa/qplatformintegration.h>
#else
#  include <QGuiApplication>
#  include <QWindow>
#endif

#include "dbustypes.h"
#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"

namespace {
Logger logger("XdgPortal");
}

XdgPortal::XdgPortal(const char* interface, QObject* parent)
    : QDBusAbstractInterface(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH, interface,
                             QDBusConnection::sessionBus(), parent) {
  MZ_COUNT_CTOR(XdgPortal);

  // Generate a unique token
  quint64 randbits = QRandomGenerator::global()->generate64();
  m_token = "mozillavpn_" + QString::number(randbits, 16);

  // See the org.freedesktop.portal.Request documentation, the request portal
  // will likely be: "/org/freedesktop/portal/desktop/request/<SENDER>/TOKEN"
  // and we should connect to it before trying to make a request.
  QDBusConnection bus = QDBusConnection::sessionBus();
  QString sender = bus.baseService().mid(1).replace('.', '_');
  constexpr const char* path = "/org/freedesktop/portal/desktop/request/%1/%2";
  setReplyPath(QString(path).arg(sender).arg(m_token));

#if MZ_DEBUG
  connect(this, &XdgPortal::xdgResponse, this, &XdgPortal::logResponse);
#endif
}

XdgPortal::~XdgPortal() { MZ_COUNT_DTOR(XdgPortal); }

QVariant XdgPortal::xdgProperty(const QString& name) const {
  QDBusMessage msg =
      QDBusMessage::createMethodCall(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH,
                                     "org.freedesktop.DBus.Properties", "Get");
  msg << interface();
  msg << name;

  QDBusReply<QDBusVariant> reply = connection().call(msg);
  if (!reply.isValid()) {
    logger.debug() << "Read" << name << "failed:" << reply.error().message();
    return QVariant();
  }
  return reply.value().variant();
}

uint XdgPortal::xdgVersion() const {
  QVariant qv = xdgProperty("version");
  if (qv.typeId() == QMetaType::UInt) {
    return qv.toUInt();
  }
  return 0;
}

void XdgPortal::setReplyPath(const QString& path) {
  if (path.isEmpty() || (path == m_replyPath)) {
    return;
  }

  QDBusConnection bus = connection();
  bus.disconnect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST,
                 "Response", this, SIGNAL(xdgResponse(uint, QVariantMap)));

  m_replyPath = path;
  bus.connect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST, "Response",
              this, SIGNAL(xdgResponse(uint, QVariantMap)));
}

void XdgPortal::logResponse(uint code, const QVariantMap& results) {
  const QString& name = metaObject()->className();
  logger.debug() << name << "response:" << code;
  for (auto i = results.cbegin(); i != results.cend(); i++) {
    logger.debug() << name << QString("%1:").arg(i.key())
                   << i.value().toString();
  }
}

// Try to find the window identifier for an XDG desktop portal request.
// https://flatpak.github.io/xdg-desktop-portal/docs/window-identifiers.html
//
// If the window identifier couldn't be determined (eg: not on Wayland/X11, or
// there is no window yet). Then this method should return an empty string.
QString XdgPortal::parentWindow() {
  if (!QmlEngineHolder::exists()) {
    return QString("");
  }
  QmlEngineHolder* holder = QmlEngineHolder::instance();
  if (!holder->hasWindow()) {
    return QString("");
  }
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
  QDesktopUnixServices* services = dynamic_cast<QDesktopUnixServices*>(
      QGuiApplicationPrivate::platformIntegration()->services());
  if (services != nullptr) {
    return services->portalWindowIdentifier(holder->window());
  }
#elif QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  QGenericUnixServices* services = dynamic_cast<QGenericUnixServices*>(
      QGuiApplicationPrivate::platformIntegration()->services());
  if (services != nullptr) {
    return services->portalWindowIdentifier(holder->window());
  }
#else
  // X11 is the only platform that we can get a window handle on prior to 6.5.0
  if (QGuiApplication::platformName() == "xcb") {
    return "x11:" + QString::number(holder->window()->winId(), 16);
  }
#endif

  // Otherwise, we don't support this windowing system.
  return QString("");
}

// Decode systemd escape characters in the cgroup names.
static QString decodeSystemdEscape(const QString& str) {
  static const QRegularExpression re("(\\\\x[0-9A-Fa-f][0-9A-Fa-f])");

  QString result = str;
  qsizetype offset = 0;
  while (offset < result.length()) {
    // Search for the next unicode escape sequence.
    QRegularExpressionMatch match = re.match(result, offset);
    if (!match.hasMatch()) {
      break;
    }

    bool okay;
    qsizetype start = match.capturedStart(0);
    QChar code = match.captured(0).sliced(2).toUShort(&okay, 16);
    if (okay && (code != 0)) {
      // Replace the matched escape sequence with the decoded character.
      result.replace(start, match.capturedLength(0), QString(code));
      offset = start + 1;
    } else {
      // If we failed to decode the character, skip passed the matched string.
      offset = match.capturedEnd(0);
    }
  }

  return result;
}

// Get our control group scope by reading /proc/self/cgroup
static QString readCgroupAppId() {
  QFile cgFile("/proc/self/cgroup");
  if (!cgFile.open(QIODeviceBase::ReadOnly | QIODeviceBase::Text)) {
    return QString();
  }
  while (true) {
    QString line = cgFile.readLine().trimmed();
    if (line.isEmpty()) {
      break;
    }
    if (!line.startsWith("0::")) {
      // Not a cgroupsv2 path.
      continue;
    }

    return XdgPortal::parseCgroupAppId(line.sliced(3));
  }

  // We failed to determine the scope.
  return QString();
}

QString XdgPortal::parseCgroupAppId(const QString& cgroup) {
  QString cgName = cgroup.split("/").last();
  if (!cgName.startsWith("app-")) {
    return QString();
  }

  // Get the suffix after the final dot.
  qsizetype dot = cgName.lastIndexOf('.');
  if (dot < 0) {
    QString();
  }
  QString suffix = cgName.sliced(dot+1);

  QString appId;
  QStringList cgSplit = cgName.first(dot).split("-");
  if (suffix == "service") {
    // Systemd services can take the forms:
    //   app[-<launcher>]-<ApplicationID>-autostart.service (deprecated)
    //   app[-<launcher>]-<ApplicationID>[@<RANDOM>].service
    if (cgSplit.last() == "autostart") {
      cgSplit.removeLast();
    }
    appId = cgSplit.last().section('@', 0, 0);
  } else if ((suffix == "scope") || (suffix == "slice")) {
    // Systemd scopes and slices can take the forms:
    //   app[-<launcher>]-<ApplicationID>-<RANDOM>.scope
    //   app[-<launcher>]-<ApplicationID>-<RANDOM>.slice
    if (cgSplit.length() < 3) {
      return QString();
    }
    appId = cgSplit.at(cgSplit.length() - 2);
  } else {
    // Otherwise, we don't recognize this systemd cgroup format.
    return QString();
  }

  // The application ID should be the last token in the scope string.
  return decodeSystemdEscape(appId);
}

void XdgPortal::setupAppScope(const QString& appId) {
  // If we're in a sandbox environment, then it should already be setup.
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("container")) {
    return;
  }

  QString cgAppId = readCgroupAppId();
  if (!cgAppId.isEmpty()) {
    // If we found an appId then we have no work to do.
    logger.info() << "Launched as app" << cgAppId;
    return;
  }

  // !! QTBUG-135928 WORKAROUND !!
  // Qt 6.8 introduced a bug where using QDBusConnection::sessionBus() before
  // QCoreApplication is created will silently break D-Bus signal connections.
  // To workaround this, spin up a separate, standalone bus for these steps.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)) && \
    (QT_VERSION < QT_VERSION_CHECK(6, 9, 2))
  QString busName = QString("%1-appid-scope-helper").arg(appId);
  QDBusConnection bus =
      QDBusConnection::connectToBus(QDBusConnection::SessionBus, busName);
  auto guard =
      qScopeGuard([busName]() { QDBusConnection::disconnectFromBus(busName); });
#else
  // If the XDG Registry portal exists, use it to advertise our application ID.
  // This is the right tool to use, but it's also very new.
  QDBusInterface xdgRegistry(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH,
                             "org.freedesktop.host.portal.Registry");
  if (xdgRegistry.property("version").toUInt() >= 1) {
    QDBusMessage msg = xdgRegistry.call("Register", appId, QVariantMap());
    if (msg.type() == QDBusMessage::ErrorMessage) {
      logger.debug() << "Registration failed:" << msg.errorMessage();
    }
    return;
  }

  QDBusConnection bus = QDBusConnection::sessionBus();
#endif

  // Request a new scope from systemd via D-Bus
  uint ownPid = (uint)getpid();
  QString newScopeName =
      QString("app-%1-%2.scope").arg(appId, QString::number(ownPid));
  SystemdUnitPropList properties;
  SystemdUnitAuxList aux;
  QList<uint> pidlist({ownPid});
  properties.append(SystemdUnitProp("PIDs", QVariant::fromValue(pidlist)));

  logger.debug() << "Creating scope:" << newScopeName;
  QDBusMessage msg = QDBusMessage::createMethodCall(
      "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
      "org.freedesktop.systemd1.Manager", "StartTransientUnit");
  msg << newScopeName;
  msg << "fail";
  msg << QVariant::fromValue(properties);
  msg << QVariant::fromValue(aux);

  QDBusMessage reply = bus.call(msg);
  if (reply.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to create scope:" << reply.errorMessage();
    return;
  }

  // And now for the gross part. StartTransientUnit() returns a systemd job that
  // will create our scope for us, and move us into it. But we have to wait for
  // that job to finish before we can proceed. However, we can't do this async
  // because we don't have a QCoreApplication setup yet. So, instead lets poll
  // our cgroup until it changes.
  for (int retries = 0; retries < 15; retries++) {
    cgAppId = readCgroupAppId();
    if (cgAppId.isEmpty()) {
      QThread::msleep(100);
      continue;
    }
    logger.debug() << "App ID updated:" << cgAppId;
    break;
  }
}
