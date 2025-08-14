/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgnotificationhandler.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QtDBus>

#include "leakdetector.h"
#include "logger.h"

constexpr const char* XDG_PORTAL_NOTIFICATION =
    "org.freedesktop.portal.Notification";

constexpr const char* ACTION_ID = "mozilla_vpn_notification";

namespace {
Logger logger("XdgNotificationHandler");
}  // namespace

// Custom D-Bus type to encode the button element.
class XdgButtonList : public QList<QVariantMap> {
 public:
  XdgButtonList() : QList<QVariantMap>(){};
  XdgButtonList(const QVariantMap& data) : QList<QVariantMap>({data}){};

  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const XdgButtonList& data) {
    args.beginArray(QMetaType::QVariantMap);
    for (const auto& entry : data) {
      args << entry;
    }
    args.endArray();
    return args;
  }

  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         XdgButtonList& data) {
    data.clear();
    args.beginArray();
    while (!args.atEnd()) {
      QVariantMap value;
      args >> value;
      data.append(value);
    }
    args.endArray();
    return args;
  }
};
Q_DECLARE_METATYPE(XdgButtonList);

XdgNotificationHandler::XdgNotificationHandler(QObject* parent)
    : SystemTrayNotificationHandler(parent) {
  MZ_COUNT_CTOR(XdgNotificationHandler);

  static bool first = true;
  if (first) {
    first = false;
    qRegisterMetaType<XdgButtonList>();
    qDBusRegisterMetaType<XdgButtonList>();
  }
}

XdgNotificationHandler::~XdgNotificationHandler() {
  MZ_COUNT_DTOR(XdgNotificationHandler);
}

void XdgNotificationHandler::initialize() {
  SystemTrayNotificationHandler::initialize();

  m_portal = new XdgPortal(XDG_PORTAL_NOTIFICATION, this);
}

void XdgNotificationHandler::notify(Message type, const QString& title,
                                    const QString& message, int timerMsec) {
  QVariantMap args;
  QVariantMap button;

  args.insert("title", title);
  args.insert("body", message);
  // TODO: Icon - or do we get this for free via our application ID?

  switch (type) {
    case UnsecuredNetwork:
      button.insert("action", ACTION_ID);
      button.insert("label", qtTrId("vpn.toggle.on"));
      break;

    case CaptivePortalBlock:
      button.insert("action", ACTION_ID);
      button.insert("label", qtTrId("vpn.toggle.off"));
      break;

    case CaptivePortalUnblock:
      button.insert("action", ACTION_ID);
      button.insert("label", qtTrId("vpn.toggle.on"));
      break;

    default:
      // For all other message, there are no actions to perform.
      break;
  }
  if (!button.isEmpty()) {
    args.insert("buttons", QVariant::fromValue(XdgButtonList(button)));
  }

  // Request the notification to be displayed
  QDBusPendingReply<> reply = m_portal->asyncCall("AddNotification", "org.mozilla.vpn", args);
  QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this,
                   [this, title, message] { emit notificationShown(title, message); });
  QObject::connect(watcher, &QDBusPendingCallWatcher::finished, watcher,
                   &QObject::deleteLater);

  m_lastMessage = type;
}

