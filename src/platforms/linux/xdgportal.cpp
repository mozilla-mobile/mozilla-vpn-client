/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgportal.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QProcessEnvironment>
#include <QRandomGenerator>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
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

XdgPortal::XdgPortal(QObject* parent) : QObject(parent) {
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

// static
uint XdgPortal::getVersion(const QString& interface) {
  QDBusInterface portal(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH, interface);
  QVariant qv = portal.property("version");
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
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

void XdgPortal::setupAppScope(const QString& appId) {
  // If we're in a sandbox environment, then it should already be setup.
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("container")) {
    return;
  }

  uint ownPid = (uint)getpid();
  QDBusInterface sdManager("org.freedesktop.systemd1",
                           "/org/freedesktop/systemd1",
                           "org.freedesktop.systemd1.Manager");

  QDBusMessage getunit = sdManager.call("GetUnitByPID", ownPid);
  if (getunit.type() == QDBusMessage::ErrorMessage) {
    logger.warning() << "Failed to get scope:" << getunit.errorMessage();
    return;
  }
  QList<QVariant> result = getunit.arguments();
  if ((getunit.type() != QDBusMessage::ReplyMessage) || result.isEmpty()) {
    logger.warning() << "Bad reply for current scope:";
    return;
  }

  // Fetch the names of the unit to figure out the appid.
  QDBusObjectPath unitPath = result.first().value<QDBusObjectPath>();
  QDBusInterface ownUnit("org.freedesktop.systemd1", unitPath.path(),
                         "org.freedesktop.systemd1.Unit");
  QStringList unitNames = ownUnit.property("Names").toStringList();

  // Use the D-Bus object path as a fallback if there are no names.
  unitNames.append(decodeSystemdEscape(unitPath.path().split('/').last()));

  // Check to see if we have a valid application scope.
  QString unitAppId;
  for (const QString& name : unitNames) {
    if (!name.endsWith(".scope")) {
      continue;
    }
    QStringList scopeSplit = name.first(name.size() - 6).split('-');
    if (scopeSplit[0] != "app") {
      continue;
    }

    // Remove the last element of the scope if it's a number.
    bool isDigit = false;
    scopeSplit.last().toULong(&isDigit);
    if (isDigit) {
      scopeSplit.removeLast();
    }

    // The appId should be the last remaining element of the scope.
    // If we found an appId then we have no work to do.
    logger.info() << "Launched as app" << scopeSplit.last();
    return;
  }

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
  QDBusObjectPath jobPath = result.first().value<QDBusObjectPath>();
  if ((msg.type() != QDBusMessage::ReplyMessage) || jobPath.path().isEmpty()) {
    logger.warning() << "Bad reply for create scope";
    return;
  }

  // And now for the gross part. StartTransientUnit() returns a systemd job that
  // will create our scope for us, and move us into it. But we have to wait for
  // that job to finish before we can proceed. However, we can't do this async
  // because we don't have a QCoreApplication setup yet. The only way I can
  // think to do this is by polling systemd until the job is no longer running.
  QDBusInterface jobInterface("org.freedesktop.systemd1", jobPath.path(),
                              "org.freedesktop.systemd1.Job");
  while (jobInterface.isValid()) {
    QString state = jobInterface.property("State").toString();
    if (state.isEmpty()) {
      logger.debug() << "Job is done exiting...";
      break;
    } else if ((state != "waiting") && (state != "running")) {
      logger.debug() << "Job is" << state << "exiting...";
      break;
    }
    logger.debug() << "Job is" << state << "waiting...";
    QThread::currentThread()->wait(100);
  }
}
