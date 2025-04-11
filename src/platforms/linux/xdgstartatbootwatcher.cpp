/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgstartatbootwatcher.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include "leakdetector.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "xdgportal.h"

namespace {
Logger logger("XdgStartAtBootWatcher");
}

XdgStartAtBootWatcher::XdgStartAtBootWatcher()
    : XdgPortal(XDG_PORTAL_BACKGROUND) {
  MZ_COUNT_CTOR(XdgStartAtBootWatcher);

  logger.debug() << "StartAtBoot watcher";

  connect(SettingsHolder::instance(), &SettingsHolder::startAtBootChanged, this,
          &XdgStartAtBootWatcher::startAtBootChanged);

  startAtBootChanged();
}

XdgStartAtBootWatcher::~XdgStartAtBootWatcher() {
  MZ_COUNT_DTOR(XdgStartAtBootWatcher);
}

void XdgStartAtBootWatcher::xdgResponse(uint response, QVariantMap results) {
  logger.debug() << "StartAtBoot responded:" << response;
  for (auto i = results.cbegin(); i != results.cend(); i++) {
    logger.debug() << "StartAtBoot" << QString("%1:").arg(i.key())
                   << i.value().toString();
  }
}

void XdgStartAtBootWatcher::startAtBootChanged() {
  bool startAtBoot = SettingsHolder::instance()->startAtBoot();

  logger.debug() << "StartAtBoot changed:" << startAtBoot;

  QStringList cmdline = {QCoreApplication::applicationFilePath(), "ui", "-m",
                         "-s"};
  QVariantMap options;
  options["autostart"] = QVariant(startAtBoot);
  options["background"] = QVariant(true);
  options["commandline"] = QVariant(cmdline);
  options["handle_token"] = QVariant(token());

  QDBusMessage call = QDBusMessage::createMethodCall(
      XDG_PORTAL_SERVICE, XDG_PORTAL_PATH, XDG_PORTAL_BACKGROUND,
      "RequestBackground");
  call << parentWindow();
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
  Q_ASSERT(call != nullptr);
  QDBusPendingReply<QDBusObjectPath> reply = *call;
  if (reply.isError()) {
    logger.error() << "Failed to set startOnBoot:" << reply.error().message();
    return;
  }

  // We need to rebind our signals if the reply path changed.
  QString path = reply.argumentAt<0>().path();
  setReplyPath(path);
}
