/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebsocketconnection.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "mozillavpn.h"
#include "notificationhandler.h"
#include "qmlengineholder.h"
#include "serveri18n.h"
#include "settingsholder.h"
#include "networkmanager.h"
#include "task.h"

#include <functional>

#include <QBuffer>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QMetaObject>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QStandardPaths>
#include <QTest>
#include <QWebSocket>

namespace {
Logger logger(LOG_INSPECTOR, "InspectorWebSocketConnection");

bool s_stealUrls = false;
bool s_forwardNetwork = false;
QUrl s_lastUrl;
QString s_updateVersion;

}  // namespace

static QObject* findObject(const QString& name) {
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

  if (!parent) {
    if (parts.length() == 1) {
      int id = qmlTypeId("Mozilla.VPN", 1, 0, qPrintable(parts[0]));
      return engine->singletonInstance<QObject*>(id);
    }
    return parent;
  }

  for (int i = 1; i < parts.length(); ++i) {
    QList<QQuickItem*> children = parent->childItems();

    bool found = false;
    for (QQuickItem* item : children) {
      if (item->objectName() == parts[i]) {
        parent = item;
        found = true;
        break;
      }
    }

    if (!found) {
      QQuickItem* contentItem =
          parent->property("contentItem").value<QQuickItem*>();
      if (!contentItem) {
        return nullptr;
      }

      QList<QQuickItem*> contentItemChildren = contentItem->childItems();

      for (QQuickItem* item : contentItemChildren) {
        if (item->objectName() == parts[i]) {
          parent = item;
          found = true;
          break;
        }
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
    String,
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
    // server-switch-notification
    WebSocketSettingCommand{
        "server-switch-notification", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setServerSwitchNotification(value ==
                                                                  "true");
        },
        []() {
          return SettingsHolder::instance()->serverSwitchNotification()
                     ? "true"
                     : "false";
        }},
    // connection-change-notification
    WebSocketSettingCommand{
        "connection-change-notification", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setConnectionChangeNotification(value ==
                                                                      "true");
        },
        []() {
          return SettingsHolder::instance()->connectionChangeNotification()
                     ? "true"
                     : "false";
        }},
    // server-unavailable-notification
    WebSocketSettingCommand{
        "server-unavailable-notification", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setServerUnavailableNotification(value ==
                                                                       "true");
        },
        []() {
          return SettingsHolder::instance()->serverUnavailableNotification()
                     ? "true"
                     : "false";
        }},

    // language
    WebSocketSettingCommand{
        "language-code", WebSocketSettingCommand::String,
        [](const QByteArray& value) {
          Localizer::instance()->setCode(QString(value));
        },
        []() { return SettingsHolder::instance()->languageCode(); }},

    // server country
    WebSocketSettingCommand{
        "current-server-country-code", WebSocketSettingCommand::String, nullptr,
        []() {
          return MozillaVPN::instance()->currentServer()->exitCountryCode();
        }},

    // server city
    WebSocketSettingCommand{
        "current-server-city", WebSocketSettingCommand::String, nullptr,
        []() {
          return MozillaVPN::instance()->currentServer()->exitCityName();
        }},

    // glean-enabled
    WebSocketSettingCommand{
        "glean-enabled", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setGleanEnabled(value == "true");
        },
        []() {
          return SettingsHolder::instance()->gleanEnabled() ? "true" : "false";
        }},

    // telemetry-policy-shown
    WebSocketSettingCommand{
        "telemetry-policy-shown", WebSocketSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setTelemetryPolicyShown(value == "true");
        },
        []() {
          return SettingsHolder::instance()->telemetryPolicyShown() ? "true"
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
                       MozillaVPN* vpn = MozillaVPN::instance();
                       Q_ASSERT(vpn);

                       vpn->reset(true);
                       vpn->hideAlert();

                       SettingsHolder* settingsHolder =
                           SettingsHolder::instance();
                       Q_ASSERT(settingsHolder);

                       // Extra cleanup for testing
                       settingsHolder->setTelemetryPolicyShown(false);

                       return QJsonObject();
                     }},
    WebSocketCommand{"fetch_network", "Enables forwarding of networkRequests",
                     0,
                     [](const QList<QByteArray>&) {
                       s_forwardNetwork = true;
                       return QJsonObject();
                     }},
    WebSocketCommand{"view_tree", "Sends a view tree", 0,
                     [](const QList<QByteArray>&) {
                       return InspectorWebSocketConnection::getViewTree();
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

    WebSocketCommand{"list", "List all properties for an object", 1,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       QString result;

                       QObject* item = findObject(arguments[1]);
                       if (!item) {
                         obj["error"] = "Object not found";
                         return obj;
                       }
                       const QMetaObject* meta = item->metaObject();
                       int start = meta->propertyOffset();
                       size_t longest = 0;

                       for (int i = start; i < meta->propertyCount(); i++) {
                         QMetaProperty mp = meta->property(i);
                         size_t namelen = strlen(mp.name());
                         if (namelen > longest) {
                           longest = namelen;
                         }
                       }

                       for (int i = start; i < meta->propertyCount(); i++) {
                         QMetaProperty mp = meta->property(i);
                         size_t padding = longest - strlen(mp.name());
                         QVariant value = mp.read(item);
                         QString name = mp.name() + QString(padding, ' ');

                         if (value.type() == QVariant::StringList) {
                           QStringList list = value.value<QStringList>();
                           if (list.isEmpty()) {
                             result += name + " =\n";
                             continue;
                           }
                           for (const QString& x : list) {
                             result += name + " = " + x + "\n";
                             name.fill(' ', longest);
                           }
                           continue;
                         }

                         result += name + " = " + value.toString() + "\n";
                       }

                       obj["value"] = result.trimmed();
                       return obj;
                     }},

    WebSocketCommand{"property", "Retrieve a property value from an object", 2,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QObject* item = findObject(arguments[1]);
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

    WebSocketCommand{"set_property", "Set a property value to an object", 4,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QVariant value;
                       if (arguments[3] == "i") {
                         value = arguments[4].toInt();
                       } else if (arguments[3] == "s") {
                         value = arguments[4];
                       } else {
                         obj["error"] = "Unsupported type. Use: i, s";
                       }

                       QObject* item = findObject(arguments[1]);
                       if (!item) {
                         obj["error"] = "Object not found";
                         return obj;
                       }

                       if (!item->setProperty(arguments[2], value)) {
                         obj["error"] = "Property is invalid";
                         return obj;
                       }

                       return obj;
                     }},

    WebSocketCommand{"click", "Click on an object", 1,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QObject* qmlobj = findObject(arguments[1]);
                       if (!qmlobj) {
                         logger.error() << "Did not find object to click on";
                         obj["error"] = "Object not found";
                         return obj;
                       }
                       QQuickItem* item = qobject_cast<QQuickItem*>(qmlobj);
                       if (!item) {
                         logger.error() << "Object is not clickable";
                         obj["error"] = "Object is not clickable";
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
    WebSocketCommand{"pushViewTo", "Push a QML View to a StackView", 2,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       QString stackViewName(arguments[1]);
                       QUrl qrcPath(arguments[2]);
                       if (!qrcPath.isValid()) {
                         obj["error"] = " Not a valid URL!";
                         logger.error() << "Not a valid URL!";
                       }

                       QObject* qmlobj = findObject(stackViewName);
                       if (qmlobj == nullptr) {
                         obj["error"] =
                             "Cant find, stackview :" + stackViewName;
                       }

                       QVariant arg = QVariant::fromValue(qrcPath.toString());

                       bool ok = QMetaObject::invokeMethod(
                           qmlobj, "debugPush", QGenericReturnArgument(),
                           Q_ARG(QVariant, arg));
                       logger.info() << "WAS OK ->" << ok;
                       return obj;
                     }},

    WebSocketCommand{"click_notification", "Click on a notification", 0,
                     [](const QList<QByteArray>&) {
                       NotificationHandler::instance()->messageClickHandle();
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

    WebSocketCommand{"set_glean_source_tags",
                     "Set Glean Source Tags (supply a comma seperated list)", 1,
                     [](const QList<QByteArray>& arguments) {
                       QStringList tags = QString(arguments[1]).split(',');
                       MozillaVPN::instance()->setGleanSourceTags(tags);
                       return QJsonObject();
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

    WebSocketCommand{
        "force_captive_portal_detection", "Simulate a captive portal detection",
        0,
        [](const QList<QByteArray>&) {
          MozillaVPN::instance()
              ->captivePortalDetection()
              ->captivePortalDetected();
          MozillaVPN::instance()->controller()->captivePortalPresent();
          return QJsonObject();
        }},

    WebSocketCommand{
        "force_unsecured_network", "Force an unsecured network detection", 0,
        [](const QList<QByteArray>&) {
          MozillaVPN::instance()->networkWatcher()->unsecuredNetwork("Dummy",
                                                                     "Dummy");
          return QJsonObject();
        }},

    WebSocketCommand{
        "force_server_unavailable",
        "Timeout all servers in a city using force_server_unavailable "
        "{countryCode} "
        "{cityCode}",
        2,
        [](const QList<QByteArray>& arguments) {
          QJsonObject obj;
          if (QString(arguments[1]) != "" && QString(arguments[2]) != "") {
            MozillaVPN::instance()
                ->controller()
                ->setCooldownForAllServersInACity(QString(arguments[1]),
                                                  QString(arguments[2]));
          } else {
            obj["error"] =
                QString("Please provide country and city codes as arguments");
          }

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

    WebSocketCommand{"hard_reset", "Hard reset (wipe all settings).", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN::instance()->hardReset();
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

                case WebSocketSettingCommand::String:
                  // Nothing to do for strings.
                  break;

                default:
                  Q_ASSERT(false);
              }

              if (!setting.m_set) {
                obj["error"] =
                    QString("Read-only settings %1").arg(QString(arguments[1]));
                return obj;
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
        "setting", "Get a setting value", 1,
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

    WebSocketCommand{"languages", "Returns a list of languages", 0,
                     [](const QList<QByteArray>&) {
                       QJsonObject obj;

                       Localizer* localizer = Localizer::instance();
                       Q_ASSERT(localizer);

                       QJsonArray languages;
                       for (const QString& language : localizer->languages()) {
                         languages.append(language);
                       }

                       obj["value"] = languages;
                       return obj;
                     }},

    WebSocketCommand{"translate", "Translate a string", 1,
                     [](const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       obj["value"] = qtTrId(arguments[1]);
                       return obj;
                     }},

    WebSocketCommand{"screen_capture", "Take a screen capture", 0,
                     [](const QList<QByteArray>&) {
                       QJsonObject obj;

                       QWindow* window = QmlEngineHolder::instance()->window();
                       if (!window) {
                         obj["error"] = "Unable to identify the window";
                         return obj;
                       }

                       QScreen* screen = window->screen();
                       if (!screen) {
                         obj["error"] = "Unable to identify the screen";
                         return obj;
                       }

                       QPixmap pixmap = screen->grabWindow(window->winId());
                       if (pixmap.isNull()) {
                         obj["error"] = "Unable to grab the window";
                         return obj;
                       }

                       QByteArray data;
                       {
                         QBuffer buffer(&data);
                         buffer.open(QIODevice::WriteOnly);
                         if (!pixmap.save(&buffer, "PNG")) {
                           obj["error"] = "Unable to save the pixmap";
                           return obj;
                         }
                       }
                       obj["type"] = "screen";
                       obj["value"] =
                           QString(data.toBase64(QByteArray::Base64Encoding));
                       return obj;
                     }},

    WebSocketCommand{
        "servers", "Returns a list of servers", 0,
        [](const QList<QByteArray>&) {
          QJsonObject obj;

          QJsonArray countryArray;
          ServerCountryModel* scm =
              MozillaVPN::instance()->serverCountryModel();
          for (const ServerCountry& country : scm->countries()) {
            QJsonArray cityArray;
            for (const ServerCity& city : country.cities()) {
              QJsonObject cityObj;
              cityObj["name"] = city.name();
              cityObj["localizedName"] =
                  ServerI18N::translateCityName(country.code(), city.name());
              cityObj["code"] = city.code();
              cityArray.append(cityObj);
            }

            QJsonObject countryObj;
            countryObj["name"] = country.name();
            countryObj["localizedName"] = ServerI18N::translateCountryName(
                country.code(), country.name());
            countryObj["code"] = country.code();
            countryObj["cities"] = cityArray;

            countryArray.append(countryObj);
          }

          obj["value"] = countryArray;
          return obj;
        }},

    WebSocketCommand{
        "reset_surveys",
        "Reset the list of triggered surveys and the installation time", 0,
        [](const QList<QByteArray>&) {
          SettingsHolder* settingsHolder = SettingsHolder::instance();
          Q_ASSERT(settingsHolder);

          settingsHolder->setInstallationTime(QDateTime::currentDateTime());
          settingsHolder->setConsumedSurveys(QStringList());

          return QJsonObject();
        }},
    WebSocketCommand{
        "dismiss_surveys", "Dismisses all surveys", 0,
        [](const QList<QByteArray>&) {
          SettingsHolder* settingsHolder = SettingsHolder::instance();
          Q_ASSERT(settingsHolder);
          auto surveys = MozillaVPN::instance()->surveyModel()->surveys();
          QStringList consumedSurveys;
          for (auto& survey : surveys) {
            consumedSurveys.append(survey.id());
          }
          settingsHolder->setInstallationTime(QDateTime::currentDateTime());
          settingsHolder->setConsumedSurveys(consumedSurveys);
          MozillaVPN::instance()->surveyModel()->dismissCurrentSurvey();
          return QJsonObject();
        }},

    WebSocketCommand{"devices", "Retrieve the list of devices", 0,
                     [](const QList<QByteArray>&) {
                       MozillaVPN* vpn = MozillaVPN::instance();
                       Q_ASSERT(vpn);

                       DeviceModel* dm = vpn->deviceModel();
                       Q_ASSERT(dm);

                       QJsonArray deviceArray;
                       for (const Device& device : dm->devices()) {
                         QJsonObject deviceObj;
                         deviceObj["name"] = device.name();
                         deviceObj["publicKey"] = device.publicKey();
                         deviceObj["currentDevice"] =
                             device.isCurrentDevice(vpn->keys());
                         deviceArray.append(deviceObj);
                       }

                       QJsonObject obj;
                       obj["value"] = deviceArray;
                       return obj;
                     }},

    WebSocketCommand{
        "reset_devices",
        "Remove all the existing devices and add the current one if needed", 0,
        [](const QList<QByteArray>&) {
          MozillaVPN* vpn = MozillaVPN::instance();
          Q_ASSERT(vpn);

          DeviceModel* dm = vpn->deviceModel();
          Q_ASSERT(dm);

          bool hasCurrentOne = false;
          for (const Device& device : dm->devices()) {
            if (device.isCurrentDevice(vpn->keys())) {
              hasCurrentOne = true;
              continue;
            }

            vpn->removeDeviceFromPublicKey(device.publicKey());
          }

          if (!hasCurrentOne) {
            vpn->addCurrentDeviceAndRefreshData();
          }

          return QJsonObject();
        }},
};

InspectorWebSocketConnection::InspectorWebSocketConnection(
    QObject* parent, QWebSocket* connection)
    : QObject(parent), m_connection(connection) {
  MVPN_COUNT_CTOR(InspectorWebSocketConnection);

#if !defined(MVPN_ANDROID) && !defined(MVPN_IOS)
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  Q_ASSERT(connection->localAddress() == QHostAddress("::ffff:127.0.0.1") ||
           connection->localAddress() == QHostAddress::LocalHost ||
           connection->localAddress() == QHostAddress::LocalHostIPv6);
#endif

  logger.debug() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QWebSocket::textMessageReceived, this,
          &InspectorWebSocketConnection::textMessageReceived);
  connect(m_connection, &QWebSocket::binaryMessageReceived, this,
          &InspectorWebSocketConnection::binaryMessageReceived);

  connect(LogHandler::instance(), &LogHandler::logEntryAdded, this,
          &InspectorWebSocketConnection::logEntryAdded);

  connect(NotificationHandler::instance(),
          &NotificationHandler::notificationShown, this,
          &InspectorWebSocketConnection::notificationShown);
  connect(NetworkManager::instance()->networkAccessManager(),
          &QNetworkAccessManager::finished, this,
          &InspectorWebSocketConnection::networkRequestFinished);
}

InspectorWebSocketConnection::~InspectorWebSocketConnection() {
  MVPN_COUNT_DTOR(InspectorWebSocketConnection);
  logger.debug() << "Connection released";
}

void InspectorWebSocketConnection::textMessageReceived(const QString& message) {
  logger.debug() << "Text message received";
  parseCommand(message.toLocal8Bit());
}

void InspectorWebSocketConnection::binaryMessageReceived(
    const QByteArray& message) {
  logger.debug() << "Binary message received";
  parseCommand(message);
}

void InspectorWebSocketConnection::parseCommand(const QByteArray& command) {
  logger.debug() << "command received:" << command;

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

void InspectorWebSocketConnection::networkRequestFinished(
    QNetworkReply* reply) {
  if (!s_forwardNetwork) {
    return;
  }
  logger.debug() << "Network Request finished";
  QJsonObject obj;
  obj["type"] = "network";
  QJsonObject request;
  QJsonObject response;

  QVariant statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  response["status"] = statusCode.isValid() ? statusCode.toInt() : -1;

  // Serialize the Response
  QJsonObject responseHeader;
  for (auto headerPair : reply->rawHeaderPairs()) {
    responseHeader[QString(headerPair.first)] = QString(headerPair.second);
  }
  response["headers"] = responseHeader;
  response["errors"] = "";
  if (reply->error() != QNetworkReply::NoError) {
    response["errors"] = reply->errorString();
  }
  response["body"] = QString(reply->readAll());

  auto qrequest = reply->request();
  // Serialize the Request
  QJsonArray requestHeaders;
  for (auto header : qrequest.rawHeaderList()) {
    requestHeaders.append(QString(header));
  }
  request["headers"] = requestHeaders;
  request["url"] = qrequest.url().toString();
  auto initator = qrequest.originatingObject();
  Task* maybe_task = dynamic_cast<Task*>(initator);
  if (maybe_task) {
    request["initiator"] = maybe_task->name();
  } else {
    request["initiator"] = getObjectClass(initator);
  }

  obj["request"] = request;
  obj["response"] = response;
  m_connection->sendTextMessage(
      QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

// static
QString InspectorWebSocketConnection::getObjectClass(const QObject* target) {
  if (target == nullptr) {
    return "unkown";
  }
  auto metaObject = target->metaObject();
  return metaObject->className();
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

// static
QJsonObject InspectorWebSocketConnection::getViewTree() {
  QJsonObject out;
  out["type"] = "qml_tree";

  QQmlApplicationEngine* engine = QmlEngineHolder::instance()->engine();
  QJsonArray viewRoots;
  for (auto& root : engine->rootObjects()) {
    QQuickWindow* window = qobject_cast<QQuickWindow*>(root);
    if (window == nullptr) {
      continue;
    }
    QQuickItem* content = window->contentItem();
    viewRoots.append(serialize(content));
  }
  out["tree"] = viewRoots;
  return out;
}

// static
QJsonObject InspectorWebSocketConnection::serialize(QQuickItem* item) {
  QJsonObject out;
  if (item == nullptr) {
    return out;
  }
  out["__class__"] = getObjectClass(item);

  // Todo: Check QObject subelements for the Layout Element
  auto metaObject = item->metaObject();
  int propertyCount = metaObject->propertyCount();
  out["__propertyCount__"] = propertyCount;
  QJsonArray props;
  for (int i = 0; i < metaObject->propertyCount(); i++) {
    auto property = metaObject->property(i);
    if (!property.isValid()) {
      continue;
    }
    QJsonObject prop;
    auto name = property.name();
    auto value = property.read(item);
    if (value.canConvert<QJsonValue>()) {
      out[name] = value.toJsonValue();
    } else if (value.canConvert<QString>()) {
      out[name] = value.toString();
    } else if (value.canConvert<QStringList>()) {
      auto list = value.toStringList();
      QJsonArray somelist;
      for (const QString& s : list) {
        somelist.append(s);
      }
      out[name] = somelist;
    } else {
      out[name] = value.typeName();
    }
  }

  QJsonArray subView;
  auto children = item->childItems();
  for (auto& c : children) {
    subView.append(serialize(c));
  }
  out["subItems"] = subView;
  return out;
}
