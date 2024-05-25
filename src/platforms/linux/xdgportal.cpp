/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgportal.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QRandomGenerator>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#  include <private/qgenericunixservices_p.h>
#  include <private/qguiapplication_p.h>
#  include <qpa/qplatformintegration.h>
#else
#  include <QGuiApplication>
#endif

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
