/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "xdgnotificationhandler.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QtDBus>

#include "constants.h"
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
  m_portal->xdgConnect("ActionInvoked", this,
                       SLOT(actionInvoked(QString, QString, QVariantMap)));
}

void XdgNotificationHandler::notify(Message type, const QString& title,
                                    const QString& message, int timerMsec) {
  Q_UNUSED(timerMsec);

  QVariantMap notify;
  QVariantMap button;
  QStringList hint("show-as-new");
  switch (type) {
    case None:
      // Treat most connection/disconnection messages as transient.
      hint.append("transient");
      break;

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
    notify.insert("buttons", QVariant::fromValue(XdgButtonList(button)));
  }
  notify.insert("title", title);
  notify.insert("body", message);
  notify.insert("display-hint", hint);

  m_lastTitle = title;
  m_lastBody = message;
  m_lastMessage = type;

  QString id = Constants::LINUX_APP_ID;
  id.append(QString(".notify-%1").arg(type));

  // Request the notification to be displayed
  m_portal->callWithCallback("AddNotification", QList<QVariant>({id, notify}),
                             this, SLOT(notifyFinished(QDBusMessage)));
}

void XdgNotificationHandler::notifyFinished(const QDBusMessage& msg) {
  if (msg.type() != QDBusMessage::ReplyMessage) {
    logger.warning() << "notification error:" << msg.errorMessage();
  } else {
    emit notificationShown(m_lastTitle, m_lastBody);
  }
}

void XdgNotificationHandler::actionInvoked(const QString& id,
                                           const QString& action,
                                           const QVariantMap& params) {
  logger.debug() << "ActionInvoked:" << id << action;
  if (id.startsWith(Constants::LINUX_APP_ID) && (action == ACTION_ID)) {
    messageClickHandle();
  }
}
