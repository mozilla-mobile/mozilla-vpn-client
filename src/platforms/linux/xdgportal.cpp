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

XdgPortal::XdgPortal(const QString& iface, QObject* parent)
    : QObject(parent), m_portal(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH, iface) {
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
}

XdgPortal::~XdgPortal() { MZ_COUNT_DTOR(XdgPortal); }

uint XdgPortal::getVersion() {
  QVariant qv = m_portal.property("version");
  if (qv.typeId() == QMetaType::UInt) {
    return qv.toUInt();
  }
  return 0;
}

void XdgPortal::setReplyPath(const QString& path) {
  if (path.isEmpty() || (path == m_replyPath)) {
    return;
  }

  QDBusConnection bus = QDBusConnection::sessionBus();
  bus.disconnect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST,
                 "Response", this, SLOT(handleDbusResponse(uint, QVariantMap)));

  m_replyPath = path;
  bus.connect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST, "Response",
              this, SLOT(handleDbusResponse(uint, QVariantMap)));
}

void XdgPortal::handleDbusResponse(uint response, QVariantMap results) {
#ifdef MZ_DEBUG
  logger.debug() << "Reply received:" << response;
  for (auto i = results.cbegin(); i != results.cend(); i++) {
    logger.debug() << QString("%1:").arg(i.key()) << i.value().toString();
  }
#endif

  emit xdgResponse(response, results);
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

// Decode systemd escape characters in the unit names.
static QString decodeSystemdEscape(const QString& str) {
  static const QRegularExpression re("(_[0-9A-Fa-f][0-9A-Fa-f])");

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
    QChar code = match.captured(0).mid(1).toUShort(&okay, 16);
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
    
    // Find the last cgroup path segment ending with ".scope"
    QStringList cgroup = line.sliced(3).split("/");
    for (auto i = cgroup.crbegin(); i != cgroup.crend(); i++) {
      if (!i->startsWith("app-") || !i->endsWith(".scope")) {
        continue;
      }

      // Parse the scope for the application ID.
      QStringList scopeSplit = i->chopped(6).split("-");

      // Remove the last element of the scope if it's a number. This likely
      // holds a PID or some other runtime identifier.
      bool isDigit = false;
      scopeSplit.last().toULong(&isDigit);
      if (isDigit) {
        scopeSplit.removeLast();
      }

      // The application ID should be the last token in the scope string.
      return scopeSplit.last();
    }
    break;
  }

  // We failed to determine the scope.
  return QString();
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

  uint ownPid = (uint)getpid();
  QDBusInterface sdManager("org.freedesktop.systemd1",
                           "/org/freedesktop/systemd1",
                           "org.freedesktop.systemd1.Manager");

  // Request a new scope from systemd via D-Bus
  QString newScopeName =
      QString("app-%1-%2.scope").arg(appId, QString::number(getpid()));
  SystemdUnitPropList properties;
  SystemdUnitAuxList aux;
  QList<uint> pidlist({ownPid});
  properties.append(SystemdUnitProp("PIDs", QVariant::fromValue(pidlist)));

  logger.debug() << "Creating scope:" << newScopeName;
  QDBusMessage msg =
      sdManager.call("StartTransientUnit", newScopeName, "fail",
                     QVariant::fromValue(properties), QVariant::fromValue(aux));
  if (msg.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to create scope:" << msg.errorMessage();
    return;
  }
  QList<QVariant> jobResult = msg.arguments();
  QDBusObjectPath jobPath = jobResult.first().value<QDBusObjectPath>();
  if ((msg.type() != QDBusMessage::ReplyMessage) || jobPath.path().isEmpty()) {
    logger.warning() << "Bad reply for create scope";
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
    logger.debug() << "Scope updated:" << cgAppId;
    break;
  }
}
