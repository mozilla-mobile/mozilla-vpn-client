/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgstartatbootwatcher.h"

#include "leakdetector.h"
#include "logger.h"
#include "settingsholder.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QRandomGenerator>

namespace {
Logger logger("XdgStartAtBootWatcher");
}

constexpr const char* XDG_PORTAL_SERVICE = "org.freedesktop.portal.Desktop";
constexpr const char* XDG_PORTAL_PATH = "/org/freedesktop/portal/desktop";
constexpr const char* XDG_PORTAL_BACKGROUND = "org.freedesktop.portal.Background";
constexpr const char* XDG_PORTAL_REQUEST = "org.freedesktop.portal.Request";

XdgStartAtBootWatcher::XdgStartAtBootWatcher() : QObject() {
  MZ_COUNT_CTOR(XdgStartAtBootWatcher);

  logger.debug() << "StartAtBoot watcher";

  connect(SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
          &XdgStartAtBootWatcher::startAtBootChanged);

  m_replyPath = xdgReplyPath();
  QDBusConnection::sessionBus().connect(XDG_PORTAL_SERVICE, m_replyPath,
                                        XDG_PORTAL_REQUEST, "Response", this,
                                        SLOT(xdgResponse(uint, QVariantMap)));

  // Generate a unique token for this application instance.
  quint64 randbits = QRandomGenerator::global()->generate64();
  m_token = "mozillavpn-" + QString::number(randbits, 16);

  startAtBootChanged();
}

XdgStartAtBootWatcher::~XdgStartAtBootWatcher() {
  MZ_COUNT_DTOR(XdgStartAtBootWatcher);
}

// See the org.freedesktop.portal.Request documentation, the request portal will
// likely take the form of /org/freedesktop/portal/desktop/request/<SENDER>/TOKEN
// and we should connect to it before trying to make a request.
QString XdgStartAtBootWatcher::xdgReplyPath() {
  QDBusConnection bus = QDBusConnection::sessionBus();
  QString sender = bus.baseService().mid(1).replace('.', '_');
  return QString("/org/freedesktop/portal/desktop/request/%1/%2").arg(sender).arg(m_token);
}

void XdgStartAtBootWatcher::xdgResponse(uint response, QVariantMap results) {
  logger.debug() << "StartAtBoot responded:" << response;
  for (auto i = results.cbegin(); i != results.cend(); i++) {
    logger.debug() << "StartAtBoot" << QString("%1:").arg(i.key()) << i.value().toString();
  }
}

void XdgStartAtBootWatcher::startAtBootChanged() {
  bool startAtBoot = SettingsHolder::instance()->startAtBoot();

  logger.debug() << "StartAtBoot changed:" << startAtBoot;

  QStringList cmdline =
      {QCoreApplication::applicationFilePath(), "ui", "-m", "-s"};
  QVariantMap options;
  options["autostart"] = QVariant(startAtBoot);
  options["background"] = QVariant(true);
  options["commandline"] = QVariant(cmdline);
  options["handle_token"] = QVariant(m_token);

  QDBusMessage call =
      QDBusMessage::createMethodCall(XDG_PORTAL_SERVICE, XDG_PORTAL_PATH,
                                     XDG_PORTAL_BACKGROUND,
                                     "RequestBackground");
  call << ""; // TODO: parent_window
  call << options;

  // Make the D-Bus call.
  QDBusConnection bus = QDBusConnection::sessionBus();
  QDBusPendingReply<QDBusObjectPath> reply = bus.asyncCall(call);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  connect(watcher, &QDBusPendingCallWatcher::finished, this,
          &XdgStartAtBootWatcher::callCompleted);
  connect(watcher, &QDBusPendingCallWatcher::finished, watcher,
          &QObject::deleteLater);
}

void XdgStartAtBootWatcher::callCompleted(QDBusPendingCallWatcher* call) {
  QDBusPendingReply<QDBusObjectPath> reply = *call;
  if (reply.isError()) {
    logger.error() << "Error received from the DBus service";
    return;
  }

  // We need to rebind our signals if the reply path changed.
  QString path = reply.argumentAt<0>().path();
  logger.debug() << "Expecting XDG response at:" << path;
  if (!path.isEmpty() && path != m_replyPath) {
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.disconnect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST,
                   "Response", this, SLOT(xdgResponse(uint, QVariantMap)));

    m_replyPath = path;
    bus.connect(XDG_PORTAL_SERVICE, m_replyPath, XDG_PORTAL_REQUEST, "Response",
                this, SLOT(xdgResponse(uint, QVariantMap)));
  }
}
