/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebsocketconnection.h"
#include "leakdetector.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "systemtrayhandler.h"

#include <functional>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QHostAddress>
#include <QQuickItem>
#include <QQuickWindow>
#include <QWebSocket>
#include <QTest>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorWebSocketConnection");

bool s_stealUrls = false;
QUrl s_lastUrl;
QString s_updateVersion;

}  // namespace

static QQuickItem* findObject(const QString& name) {
  QStringList parts = name.split("/");
  Q_ASSERT(!parts.isEmpty());

  QQuickItem* parent = nullptr;
  QQmlApplicationEngine* engine = QmlEngineHolder::instance()->engine();
  for (QObject* rootObject : engine->rootObjects()) {
    if (!rootObject) {
      continue;
    }

    parent = rootObject->findChild<QQuickItem*>(parts[0]);
    if (parent) {
      break;
    }
  }

  if (!parent || parts.length() == 1) {
    return parent;
  }

  for (int i = 1; i < parts.length(); ++i) {
    QQuickItem* contentItem =
        parent->property("contentItem").value<QQuickItem*>();
    if (!contentItem) {
      return nullptr;
    }

    QList<QQuickItem*> contentItemChildren = contentItem->childItems();

    bool found = false;
    for (QQuickItem* item : contentItemChildren) {
      if (item->objectName() == parts[i]) {
        parent = item;
        found = true;
        break;
      }
    }

    if (!found) {
      return nullptr;
    }
  }

  return parent;
}

struct WebSocketSettingCommand {
  QString m_settingName;

  enum {
    Boolean,
  } m_type;

  std::function<void(const QByteArray&)> m_set;
  std::function<QJsonValue()> m_get;
};

// The list of settings exposed to the websocket.
static QList<WebSocketSettingCommand> s_settingCommands{
    // Unsecured-network-alert
    WebSocketSettingCommand{
        "unsecured-network-alert", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setUnsecuredNetworkAlert(value == "true");
        },
        []() {
          return SettingsHolder::instance()->unsecuredNetworkAlert() ? "true"
                                                                     : "false";
        }},

    // Captive portal
    WebSocketSettingCommand{
        "captive-portal-alert", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setCaptivePortalAlert(value == "true");
        },
        []() {
          return SettingsHolder::instance()->captivePortalAlert() ? "true"
                                                                  : "false";
        }},

    // start at boot
    WebSocketSettingCommand{
        "start-at-boot", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setStartAtBoot(value == "true");
        },
        []() {
          return SettingsHolder::instance()->startAtBoot() ? "true" : "false";
        }},

    // ipv6
    WebSocketSettingCommand{
        "ipv6-enabled", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setIpv6Enabled(value == "true");
        },
        []() {
          return SettingsHolder::instance()->ipv6Enabled() ? "true" : "false";
        }},

    // local area network access
    WebSocketSettingCommand{
        "local-network-access", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setLocalNetworkAccess(value == "true");
        },
        []() {
          return SettingsHolder::instance()->localNetworkAccess() ? "true"
                                                                  : "false";
        }},

};

struct WebSocketCommand {
  QString m_commandName;
  QString m_commandDescription;
  int32_t m_arguments;
  std::function<QJsonObject(const QList<QByteArray>&)> m_callback;
};

static QList<WebSocketCommand> s_commands{
    WebSocketCommand{"help", "The help menu", 0,
                     [](const QList<QByteArray>&) {
                       QJsonObject obj;

                       QString value;

                       {
                         QTextStream out(&value);
                         for (const WebSocketCommand& command : s_commands) {
                           out << command.m_commandName << '\t'
                               << command.m_commandDescription << Qt::endl;
                         }
                       }

                       obj["value"] = value;
                       return obj;
                     }},

    WebSocketCommand{"reset", "Reset the app", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->reset(true);
                       return QJsonObject();
                     }},

    WebSocketCommand{"quit", "Quit the app", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->controller()->quit();
                       return QJsonObject();
                     }},

    WebSocketCommand{"has", "Check if an object exists", 1,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       obj["value"] = !!findObject(arguments[1]);
                       return obj;
                     }},

    WebSocketCommand{"property", "Retrieve a property value from an object", 2,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QQuickItem* item = findObject(arguments[1]);
                       if (!item) {
                         obj["error"] = "Object not found";
                         return obj;
                       }

                       QVariant property = item->property(arguments[2]);
                       if (!property.isValid()) {
                         obj["error"] = "Property is invalid";
                         return obj;
                       }

                       obj["value"] = property.toString();
                       return obj;
                     }},

    WebSocketCommand{"click", "Click on an object", 1,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QQuickItem* item = findObject(arguments[1]);
                       if (!item) {
                         obj["error"] = "Object not found";
                         return obj;
                       }

                       QPointF pointF = item->mapToScene(QPoint(0, 0));
                       QPoint point = pointF.toPoint();
                       point.rx() += item->width() / 2;
                       point.ry() += item->height() / 2;
                       QTest::mouseClick(item->window(), Qt::LeftButton,
                                         Qt::NoModifier, point);

                       return obj;
                     }},

    WebSocketCommand{"click_notification", "Click on a notification", 0,
                     [](const QList<QByteArray>&) {
                       SystemTrayHandler::instance()->messageClickHandle();
                       return QJsonObject();
                     }},

    WebSocketCommand{
        "stealurls",
        "Do not open the URLs in browser and expose them via webSocket", 0,
        [](const QList<QByteArray>&) {
          s_stealUrls = true;
          return QJsonObject();
        }},

    WebSocketCommand{"lasturl", "Retrieve the last opened URL", 0,
                     [](const QList<QByteArray>&) {
                       QJsonObject obj;
                       obj["value"] = s_lastUrl.toString();
                       return obj;
                     }},

    WebSocketCommand{"force_update_check", "Force a version update check", 1,
                     [](const QList<QByteArray>& arguments) {
                       s_updateVersion = arguments[1];
                       MozillaVPN::instance()->releaseMonitor()->runSoon();
                       return QJsonObject();
                     }},

    WebSocketCommand{"force_captive_portal_check",
                     "Force a captive portal check", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()
                           ->captivePortalDetection()
                           ->detectCaptivePortal();
                       return QJsonObject();
                     }},

    WebSocketCommand{"force_captive_portal_detection",
                     "Simulate a captive portal detection", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()
                           ->captivePortalDetection()
                           ->captivePortalDetected();
                       return QJsonObject();
                     }},

    WebSocketCommand{
        "force_unsecured_network", "Force an unsecured network detection", 0,
        [](const QList<QByteArray>&) {
          MozillaVPN::instance()->networkWatcher()->unsecuredNetwork("Dummy",
                                                                     "Dummy");
          return QJsonObject();
        }},

    WebSocketCommand{"activate", "Activate the VPN", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->activate();
                       return QJsonObject();
                     }},

    WebSocketCommand{"deactivate", "Deactivate the VPN", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->deactivate();
                       return QJsonObject();
                     }},

    WebSocketCommand{"force_heartbeat_failure", "Force a heartbeat failure", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->heartbeatCompleted(
                           false /* success */);
                       return QJsonObject();
                     }},

    WebSocketCommand{"logout", "Logout the user", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->logout();
                       return QJsonObject();
                     }},

    WebSocketCommand{
        "set_setting", "Set a setting", 2,
        [](const QList<QByteArray>& arguments) {
          QJsonObject obj;

          for (const WebSocketSettingCommand& setting : s_settingCommands) {
            if (arguments[1] == setting.m_settingName) {
              switch (setting.m_type) {
                case WebSocketSettingCommand::Boolean:
                  if (arguments[2] != "true" && arguments[2] != "false") {
                    obj["error"] =
                        QString("Expected boolean (true/false) for settings %1")
                            .arg(QString(arguments[1]));
                    return obj;
                  }

                  break;

                default:
                  Q_ASSERT(false);
              }

              setting.m_set(arguments[2]);
              return obj;
            }
          }

          QStringList settings;
          for (const WebSocketSettingCommand& setting : s_settingCommands) {
            settings.append(setting.m_settingName);
          }

          obj["error"] = QString("Invalid settings. The options are: %1")
                             .arg(settings.join(", "));
          return obj;
        }},

    WebSocketCommand{
        "get_setting", "Get a setting value", 1,
        [](const QList<QByteArray>& arguments) {
          QJsonObject obj;

          for (const WebSocketSettingCommand& setting : s_settingCommands) {
            if (arguments[1] == setting.m_settingName) {
              obj["value"] = setting.m_get();
              return obj;
            }
          }

          QStringList settings;
          for (const WebSocketSettingCommand& setting : s_settingCommands) {
            settings.append(setting.m_settingName);
          }

          obj["error"] = QString("Invalid settings. The options are: %1")
                             .arg(settings.join(", "));
          return obj;
        }},
};

InspectorWebSocketConnection::InspectorWebSocketConnection(
    QObject* parent, QWebSocket* connection)
    : QObject(parent), m_connection(connection) {
  MVPN_COUNT_CTOR(InspectorWebSocketConnection);

  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  Q_ASSERT(connection->localAddress() == QHostAddress("::ffff:127.0.0.1") ||
           connection->localAddress() == QHostAddress::LocalHost ||
           connection->localAddress() == QHostAddress::LocalHostIPv6);

  logger.log() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QWebSocket::textMessageReceived, this,
          &InspectorWebSocketConnection::textMessageReceived);
  connect(m_connection, &QWebSocket::binaryMessageReceived, this,
          &InspectorWebSocketConnection::binaryMessageReceived);

  connect(LogHandler::instance(), &LogHandler::logEntryAdded, this,
          &InspectorWebSocketConnection::logEntryAdded);

  connect(SystemTrayHandler::instance(), &SystemTrayHandler::notificationShown,
          this, &InspectorWebSocketConnection::notificationShown);
}

InspectorWebSocketConnection::~InspectorWebSocketConnection() {
  MVPN_COUNT_DTOR(InspectorWebSocketConnection);
  logger.log() << "Connection released";
}

void InspectorWebSocketConnection::textMessageReceived(const QString& message) {
  logger.log() << "Text message received";
  parseCommand(message.toLocal8Bit());
}

void InspectorWebSocketConnection::binaryMessageReceived(
    const QByteArray& message) {
  logger.log() << "Binary message received";
  parseCommand(message);
}

void InspectorWebSocketConnection::parseCommand(const QByteArray& command) {
  logger.log() << "command received:" << command;

  if (command.isEmpty()) {
    return;
  }

  QList<QByteArray> parts = command.split(' ');
  Q_ASSERT(!parts.isEmpty());

  QString cmdName = parts[0].trimmed();

  for (const WebSocketCommand& command : s_commands) {
    if (cmdName == command.m_commandName) {
      if (parts.length() != command.m_arguments + 1) {
        QJsonObject obj;
        obj["type"] = command.m_commandName;
        obj["error"] = QString("too many arguments (%1 expected)")
                           .arg(command.m_arguments);
        m_connection->sendTextMessage(
            QJsonDocument(obj).toJson(QJsonDocument::Compact));
        return;
      }

      QJsonObject obj = command.m_callback(parts);
      obj["type"] = command.m_commandName;
      m_connection->sendTextMessage(
          QJsonDocument(obj).toJson(QJsonDocument::Compact));
      return;
    }
  }

  QJsonObject obj;
  obj["type"] = "unknown";
  obj["error"] = "invalid command";
  m_connection->sendTextMessage(
      QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void InspectorWebSocketConnection::logEntryAdded(const QByteArray& log) {
  // No logger here to avoid loops!

  QJsonObject obj;
  obj["type"] = "log";
  obj["value"] = QString(log).trimmed();
  m_connection->sendTextMessage(
      QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void InspectorWebSocketConnection::notificationShown(const QString& title,
                                                     const QString& message) {
  QJsonObject obj;
  obj["type"] = "notification";
  obj["title"] = title;
  obj["message"] = message;
  m_connection->sendTextMessage(
      QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

// static
void InspectorWebSocketConnection::setLastUrl(const QUrl& url) {
  s_lastUrl = url;
}

// static
bool InspectorWebSocketConnection::stealUrls() { return s_stealUrls; }

// static
QString InspectorWebSocketConnection::appVersionForUpdate() {
  if (s_updateVersion.isEmpty()) {
    return APP_VERSION;
  }

  return s_updateVersion;
}
