/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhandler.h"
#include "addons/manager/addonmanager.h"
#include "constants.h"
#include "controller.h"
#include "externalophandler.h"
#include "frontend/navigator.h"
#include "inspectoritempicker.h"
#include "inspectorutils.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "models/feature.h"
#include "models/featuremodel.h"
#include "mozillavpn.h"
#include "networkmanager.h"
#include "notificationhandler.h"
#include "profileflow.h"
#include "qmlengineholder.h"
#include "serveri18n.h"
#include "settingsholder.h"
#include "task.h"
#include "urlopener.h"
#include "websocket/pushmessage.h"

#include <functional>

#include <QBuffer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaObject>
#include <QNetworkAccessManager>
#include <QPixmap>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QTest>

#ifdef MVPN_WASM
#  include "platforms/wasm/wasminspector.h"
#else
#  include "inspectorwebsocketserver.h"

#  include <QCoreApplication>
#endif

#ifdef MVPN_ANDROID
#  include "platforms/android/androidvpnactivity.h"
#endif

namespace {
Logger logger(LOG_INSPECTOR, "InspectorHandler");

bool s_stealUrls = false;
bool s_forwardNetwork = false;
bool s_mockFreeTrial = false;
bool s_forceRTL = false;

QString s_updateVersion;
QStringList s_pickedItems;
bool s_pickedItemsSet = false;

InspectorItemPicker* s_itemPicker = nullptr;
}  // namespace

struct InspectorSettingCommand {
  QString m_settingName;

  enum {
    Boolean,
    String,
  } m_type;

  std::function<void(const QByteArray&)> m_set;
  std::function<QJsonValue()> m_get;
};

// The list of settings exposed to the inspector.
static QList<InspectorSettingCommand> s_settingCommands{
    // Unsecured-network-alert
    InspectorSettingCommand{
        "unsecured-network-alert", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setUnsecuredNetworkAlert(value == "true");
        },
        []() {
          return SettingsHolder::instance()->unsecuredNetworkAlert() ? "true"
                                                                     : "false";
        }},

    // Captive portal
    InspectorSettingCommand{
        "captive-portal-alert", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setCaptivePortalAlert(value == "true");
        },
        []() {
          return SettingsHolder::instance()->captivePortalAlert() ? "true"
                                                                  : "false";
        }},

    // start at boot
    InspectorSettingCommand{
        "start-at-boot", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setStartAtBoot(value == "true");
        },
        []() {
          return SettingsHolder::instance()->startAtBoot() ? "true" : "false";
        }},

    // local area network access
    InspectorSettingCommand{
        "local-network-access", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setLocalNetworkAccess(value == "true");
        },
        []() {
          return SettingsHolder::instance()->localNetworkAccess() ? "true"
                                                                  : "false";
        }},
    // server-switch-notification
    InspectorSettingCommand{
        "server-switch-notification", InspectorSettingCommand::Boolean,
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
    InspectorSettingCommand{
        "connection-change-notification", InspectorSettingCommand::Boolean,
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
    InspectorSettingCommand{
        "server-unavailable-notification", InspectorSettingCommand::Boolean,
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
    InspectorSettingCommand{
        "language-code", InspectorSettingCommand::String,
        [](const QByteArray& value) {
          Localizer::instance()->setCode(QString(value));
        },
        []() { return SettingsHolder::instance()->languageCode(); }},

    // server country
    InspectorSettingCommand{
        "current-server-country-code", InspectorSettingCommand::String, nullptr,
        []() {
          return MozillaVPN::instance()->currentServer()->exitCountryCode();
        }},

    // server city
    InspectorSettingCommand{
        "current-server-city", InspectorSettingCommand::String, nullptr,
        []() {
          return MozillaVPN::instance()->currentServer()->exitCityName();
        }},

    // glean-enabled
    InspectorSettingCommand{
        "glean-enabled", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setGleanEnabled(value == "true");
        },
        []() {
          return SettingsHolder::instance()->gleanEnabled() ? "true" : "false";
        }},

    // telemetry-policy-shown
    InspectorSettingCommand{
        "telemetry-policy-shown", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setTelemetryPolicyShown(value == "true");
        },
        []() {
          return SettingsHolder::instance()->telemetryPolicyShown() ? "true"
                                                                    : "false";
        }},

    // tips-and-tricks-intro-shown
    InspectorSettingCommand{
        "tips-and-tricks-intro-shown", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setTipsAndTricksIntroShown(value ==
                                                                 "true");
        },
        []() {
          return SettingsHolder::instance()->tipsAndTricksIntroShown()
                     ? "true"
                     : "false";
        }},

    InspectorSettingCommand{
        "addon/customServer", InspectorSettingCommand::Boolean,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setAddonCustomServer(value == "true");
        },
        []() {
          return SettingsHolder::instance()->addonCustomServer() ? "true"
                                                                 : "false";
        }},

    InspectorSettingCommand{
        "addon/customServerAddress", InspectorSettingCommand::String,
        [](const QByteArray& value) {
          SettingsHolder::instance()->setAddonCustomServerAddress(value);
        },
        []() {
          return SettingsHolder::instance()->addonCustomServerAddress();
        }},
};

struct InspectorCommand {
  QString m_commandName;
  QString m_commandDescription;
  int32_t m_arguments;
  std::function<QJsonObject(InspectorHandler*, const QList<QByteArray>&)>
      m_callback;
};

static QList<InspectorCommand> s_commands{
    InspectorCommand{"help", "The help menu", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonObject obj;

                       QString value;

                       {
                         QTextStream out(&value);
                         for (const InspectorCommand& command : s_commands) {
                           out << command.m_commandName << '\t'
                               << command.m_commandDescription << Qt::endl;
                         }
                       }

                       obj["value"] = value;
                       return obj;
                     }},

    InspectorCommand{"reset", "Reset the app", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN* vpn = MozillaVPN::instance();
                       Q_ASSERT(vpn);

                       vpn->reset(true);
                       ErrorHandler::instance()->hideAlert();

                       SettingsHolder* settingsHolder =
                           SettingsHolder::instance();
                       Q_ASSERT(settingsHolder);

                       // Extra cleanup for testing
                       settingsHolder->setTelemetryPolicyShown(false);

                       return QJsonObject();
                     }},
    InspectorCommand{"fetch_network", "Enables forwarding of networkRequests",
                     0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       s_forwardNetwork = true;
                       return QJsonObject();
                     }},
    InspectorCommand{"view_tree", "Sends a view tree", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       return InspectorHandler::getViewTree();
                     }},

    InspectorCommand{"quit", "Quit the app", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->controller()->quit();
                       return QJsonObject();
                     }},

    InspectorCommand{"pick", "Wait for a click to select an element", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       if (!s_itemPicker) {
                         s_itemPicker = new InspectorItemPicker(qApp);
                         qApp->installEventFilter(s_itemPicker);
                       }
                       return QJsonObject();
                     }},

    InspectorCommand{"picked", "Retrieve what has been selected with a click",
                     0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonArray array;
                       for (const QString& element : s_pickedItems) {
                         array.append(element);
                       }

                       QJsonObject obj;
                       obj["value"] = array;
                       obj["clicked"] = s_pickedItemsSet;

                       s_pickedItemsSet = false;
                       s_pickedItems.clear();
                       return obj;
                     }},

    InspectorCommand{"has", "Check if an object exists", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       obj["value"] =
                           !!InspectorUtils::findObject(arguments[1]);
                       return obj;
                     }},

    InspectorCommand{"list", "List all properties for an object", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       QString result;

                       QObject* item = InspectorUtils::findObject(arguments[1]);
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

                         if (value.typeId() == QVariant::StringList) {
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

    InspectorCommand{"property", "Retrieve a property value from an object", 2,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QObject* item = InspectorUtils::findObject(arguments[1]);
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

    InspectorCommand{"set_property", "Set a property value to an object", 4,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QVariant value;
                       if (arguments[3] == "i") {
                         value = arguments[4].toInt();
                       } else if (arguments[3] == "s") {
                         value = arguments[4];
                       } else {
                         obj["error"] = "Unsupported type. Use: i, s";
                       }

                       QObject* item = InspectorUtils::findObject(arguments[1]);
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

    InspectorCommand{"click", "Click on an object", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QObject* qmlobj =
                           InspectorUtils::findObject(arguments[1]);
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
    InspectorCommand{
        "pushViewTo", "Push a QML View to a StackView", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;
          QString stackViewName(arguments[1]);
          QUrl qrcPath(arguments[2]);
          if (!qrcPath.isValid()) {
            obj["error"] = " Not a valid URL!";
            logger.error() << "Not a valid URL!";
          }

          QObject* qmlobj = InspectorUtils::findObject(stackViewName);
          if (qmlobj == nullptr) {
            obj["error"] = "Cant find, stackview :" + stackViewName;
          }

          QVariant arg = QVariant::fromValue(qrcPath.toString());

          bool ok = QMetaObject::invokeMethod(qmlobj, "debugPush",
                                              QGenericReturnArgument(),
                                              Q_ARG(QVariant, arg));
          logger.info() << "WAS OK ->" << ok;
          return obj;
        }},

    InspectorCommand{"click_notification", "Click on a notification", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       NotificationHandler::instance()->messageClickHandle();
                       return QJsonObject();
                     }},

    InspectorCommand{
        "stealurls",
        "Do not open the URLs in browser and expose them via inspector", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
          s_stealUrls = true;
          return QJsonObject();
        }},
    InspectorCommand{"mockFreeTrial",
                     "Force the UI to show 7 day trial on 1 year plan", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       s_mockFreeTrial = true;
                       return QJsonObject();
                     }},

    InspectorCommand{"lasturl", "Retrieve the last opened URL", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonObject obj;
                       obj["value"] = UrlOpener::instance()->lastUrl();
                       return obj;
                     }},

    InspectorCommand{"set_glean_source_tags",
                     "Set Glean Source Tags (supply a comma seperated list)", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QStringList tags = QString(arguments[1]).split(',');
                       emit MozillaVPN::instance()->setGleanSourceTags(tags);
                       return QJsonObject();
                     }},

    InspectorCommand{"force_update_check", "Force a version update check", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       s_updateVersion = arguments[1];
                       MozillaVPN::instance()->releaseMonitor()->runSoon();
                       return QJsonObject();
                     }},
    InspectorCommand{"force_captive_portal_check",
                     "Force a captive portal check", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()
                           ->captivePortalDetection()
                           ->detectCaptivePortal();
                       return QJsonObject();
                     }},

    InspectorCommand{
        "force_captive_portal_detection", "Simulate a captive portal detection",
        0,
        [](InspectorHandler*, const QList<QByteArray>&) {
          MozillaVPN::instance()
              ->captivePortalDetection()
              ->captivePortalDetected();
          MozillaVPN::instance()->controller()->captivePortalPresent();
          return QJsonObject();
        }},

    InspectorCommand{
        "force_unsecured_network", "Force an unsecured network detection", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
          MozillaVPN::instance()->networkWatcher()->unsecuredNetwork("Dummy",
                                                                     "Dummy");
          return QJsonObject();
        }},

    InspectorCommand{
        "force_server_unavailable",
        "Timeout all servers in a city using force_server_unavailable "
        "{countryCode} "
        "{cityCode}",
        2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
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

    InspectorCommand{
        "force_subscription_management_reauthentication",
        "Force re-authentication for the subscription management view", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
          MozillaVPN::instance()->profileFlow()->setForceReauthFlow(true);
          return QJsonObject();
        }},

    InspectorCommand{"activate", "Activate the VPN", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->activate();
                       return QJsonObject();
                     }},

    InspectorCommand{"deactivate", "Deactivate the VPN", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->deactivate();
                       return QJsonObject();
                     }},

    InspectorCommand{"force_heartbeat_failure", "Force a heartbeat failure", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->heartbeatCompleted(
                           false /* success */);
                       return QJsonObject();
                     }},

    InspectorCommand{"hard_reset", "Hard reset (wipe all settings).", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->hardReset();
                       return QJsonObject();
                     }},

    InspectorCommand{"logout", "Logout the user", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN::instance()->logout();
                       return QJsonObject();
                     }},

    InspectorCommand{
        "set_setting", "Set a setting", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          for (const InspectorSettingCommand& setting : s_settingCommands) {
            if (arguments[1] == setting.m_settingName) {
              switch (setting.m_type) {
                case InspectorSettingCommand::Boolean:
                  if (arguments[2] != "true" && arguments[2] != "false") {
                    obj["error"] =
                        QString("Expected boolean (true/false) for settings %1")
                            .arg(QString(arguments[1]));
                    return obj;
                  }

                  break;

                case InspectorSettingCommand::String:
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
          for (const InspectorSettingCommand& setting : s_settingCommands) {
            settings.append(setting.m_settingName);
          }

          obj["error"] = QString("Invalid settings. The options are: %1")
                             .arg(settings.join(", "));
          return obj;
        }},

    InspectorCommand{
        "setting", "Get a setting value", 1,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          for (const InspectorSettingCommand& setting : s_settingCommands) {
            if (arguments[1] == setting.m_settingName) {
              obj["value"] = setting.m_get();
              return obj;
            }
          }

          QStringList settings;
          for (const InspectorSettingCommand& setting : s_settingCommands) {
            settings.append(setting.m_settingName);
          }

          obj["error"] = QString("Invalid settings. The options are: %1")
                             .arg(settings.join(", "));
          return obj;
        }},

    InspectorCommand{"settings_filename", "Get the setting filename", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonObject obj;
                       obj["value"] =
                           SettingsHolder::instance()->settingsFileName();
                       return obj;
                     }},

    InspectorCommand{"languages", "Returns a list of languages", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
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

    InspectorCommand{"guides", "Returns a list of guide title ids", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonObject obj;

                       AddonManager* am = AddonManager::instance();
                       Q_ASSERT(am);

                       QJsonArray guides;
                       am->forEach([&](Addon* addon) {
                         if (addon->type() == "guide") {
                           guides.append(addon->id());
                         }
                       });

                       obj["value"] = guides;
                       return obj;
                     }},

    InspectorCommand{"translate", "Translate a string", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       obj["value"] = qtTrId(arguments[1]);
                       return obj;
                     }},

    InspectorCommand{"screen_capture", "Take a screen capture", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
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

    InspectorCommand{
        "servers", "Returns a list of servers", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
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
#ifdef MVPN_ANDROID
    InspectorCommand{"android_daemon",
                     "Send a request to the Daemon {type} {args}", 2,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       auto activity = AndroidVPNActivity::instance();
                       Q_ASSERT(activity);
                       auto type = QString(arguments[1]);
                       auto json = QString(arguments[2]);

                       ServiceAction a = (ServiceAction)type.toInt();
                       AndroidVPNActivity::sendToService(a, json);
                       return QJsonObject();
                     }},
#endif

    InspectorCommand{"devices", "Retrieve the list of devices", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
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

    InspectorCommand{
        "reset_devices",
        "Remove all the existing devices and add the current one if needed", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
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

    InspectorCommand{"public_key",
                     "Retrieve the public key of the current device", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MozillaVPN* vpn = MozillaVPN::instance();
                       Q_ASSERT(vpn);

                       Keys* keys = vpn->keys();
                       Q_ASSERT(keys);

                       QJsonObject obj;
                       obj["value"] = keys->publicKey();
                       return obj;
                     }},

    InspectorCommand{"is_feature_flipped_on",
                     "Check if a feature is flipped on", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString featureName = arguments[1];
                       auto const settings = SettingsHolder::instance();
                       QStringList flags = settings->featuresFlippedOn();

                       QJsonObject obj;
                       obj["value"] = flags.contains(featureName);
                       return obj;
                     }},

    InspectorCommand{"is_feature_flipped_off",
                     "Check if a feature is flipped off", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString featureName = arguments[1];
                       auto const settings = SettingsHolder::instance();
                       QStringList flags = settings->featuresFlippedOff();

                       QJsonObject obj;
                       obj["value"] = flags.contains(featureName);
                       return obj;
                     }},

    InspectorCommand{"flip_on_feature", "Flip On a feature", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString featureName = arguments[1];
                       const Feature* feature = Feature::getOrNull(featureName);
                       if (!feature) {
                         QJsonObject obj;
                         obj["error"] = "Feature does not exist";
                         return obj;
                       }

                       if (!feature->isSupported()) {
                         FeatureModel::instance()->toggle(arguments[1]);
                       }
                       return QJsonObject();
                     }},

    InspectorCommand{"flip_off_feature", "Flip Off a feature", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString featureName = arguments[1];
                       const Feature* feature = Feature::getOrNull(featureName);
                       if (!feature) {
                         QJsonObject obj;
                         obj["error"] = "Feature does not exist";
                         return obj;
                       }

                       if (feature->isSupported()) {
                         FeatureModel::instance()->toggle(arguments[1]);
                       }
                       return QJsonObject();
                     }},

    InspectorCommand{"load_addon_manifest", "Load an add-on", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       // This is a debugging method. We don't need to compute
                       // the hash of the addon because we will not be able to
                       // find it in the addon index.
                       obj["value"] =
                           AddonManager::instance()->loadManifest(arguments[1]);
                       return obj;
                     }},

    InspectorCommand{"unload_addon", "Unload an add-on", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       AddonManager::instance()->unload(arguments[1]);
                       return QJsonObject();
                     }},

    InspectorCommand{"back_button_clicked",
                     "Simulate an android back-button clicked", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       Navigator::instance()->eventHandled();
                       return QJsonObject();
                     }},

    InspectorCommand{
        "send_push_message_device_deleted",
        "Simulate the receiving of a push-message type device-deleted", 1,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject payload;
          payload["publicKey"] = QString(arguments[1]);

          QJsonObject msg;
          msg["type"] = "DEVICE_DELETED";
          msg["payload"] = payload;

          PushMessage message(QJsonDocument(msg).toJson());
          message.executeAction();
          return QJsonObject();
        }},

    InspectorCommand{"set_installation_time", "Set the installation time", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       qint64 epoch = arguments[1].toLongLong();
                       SettingsHolder::instance()->setInstallationTime(
                           QDateTime::fromSecsSinceEpoch(epoch));
                       return QJsonObject();
                     }},

    InspectorCommand{"force_rtl", "Force RTL layout", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       s_forceRTL = true;
                       emit Localizer::instance()->codeChanged();
                       return QJsonObject();
                     }},
};

// static
void InspectorHandler::initialize() {
#ifdef MVPN_WASM
  WasmInspector::instance();
#else
  if (!Constants::inProduction()) {
    InspectorWebSocketServer* inspectWebSocketServer =
        new InspectorWebSocketServer(qApp);
    QObject::connect(MozillaVPN::instance()->controller(),
                     &Controller::readyToQuit, inspectWebSocketServer,
                     &InspectorWebSocketServer::close);
  }
#endif
}

InspectorHandler::InspectorHandler(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(InspectorHandler);

  connect(LogHandler::instance(), &LogHandler::logEntryAdded, this,
          &InspectorHandler::logEntryAdded);

  connect(NotificationHandler::instance(),
          &NotificationHandler::notificationShown, this,
          &InspectorHandler::notificationShown);
  connect(NetworkManager::instance()->networkAccessManager(),
          &QNetworkAccessManager::finished, this,
          &InspectorHandler::networkRequestFinished);
}

InspectorHandler::~InspectorHandler() { MVPN_COUNT_DTOR(InspectorHandler); }

void InspectorHandler::recv(const QByteArray& command) {
  logger.debug() << "command received:" << command;

  if (command.isEmpty()) {
    return;
  }

  QList<QByteArray> parts = command.split(' ');
  Q_ASSERT(!parts.isEmpty());

  QString cmdName = parts[0].trimmed();

  for (const InspectorCommand& command : s_commands) {
    if (cmdName == command.m_commandName) {
      if (parts.length() != command.m_arguments + 1) {
        QJsonObject obj;
        obj["type"] = command.m_commandName;
        obj["error"] = QString("too many arguments (%1 expected)")
                           .arg(command.m_arguments);
        send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        return;
      }

      QJsonObject obj = command.m_callback(this, parts);
      obj["type"] = command.m_commandName;
      send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
      return;
    }
  }

  QJsonObject obj;
  obj["type"] = "unknown";
  obj["error"] = "invalid command";
  send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void InspectorHandler::logEntryAdded(const QByteArray& log) {
  // No logger here to avoid loops!

  QJsonObject obj;
  obj["type"] = "log";
  obj["value"] = QString(log).trimmed();
  send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void InspectorHandler::notificationShown(const QString& title,
                                         const QString& message) {
  QJsonObject obj;
  obj["type"] = "notification";
  obj["title"] = title;
  obj["message"] = message;
  send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void InspectorHandler::networkRequestFinished(QNetworkReply* reply) {
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
  for (const auto& headerPair : reply->rawHeaderPairs()) {
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
  for (const auto& header : qrequest.rawHeaderList()) {
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
  send(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

// static
QString InspectorHandler::getObjectClass(const QObject* target) {
  if (target == nullptr) {
    return "unkown";
  }
  auto metaObject = target->metaObject();
  return metaObject->className();
}

// static
bool InspectorHandler::stealUrls() { return s_stealUrls; }

// static
bool InspectorHandler::mockFreeTrial() { return s_mockFreeTrial; }

// static
bool InspectorHandler::forceRTL() { return s_forceRTL; }

// static
QString InspectorHandler::appVersionForUpdate() {
  if (s_updateVersion.isEmpty()) {
    return Constants::versionString();
  }

  return s_updateVersion;
}

// static
QJsonObject InspectorHandler::getViewTree() {
  QJsonObject out;
  out["type"] = "qml_tree";

  QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
      QmlEngineHolder::instance()->engine());
  if (!engine) {
    return out;
  }

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
QJsonObject InspectorHandler::serialize(QQuickItem* item) {
  QJsonObject out;
  if (item == nullptr) {
    return out;
  }
  out["__class__"] = getObjectClass(item);

  // Todo: Check QObject subelements for the Layout Element
  auto metaObject = item->metaObject();
  int propertyCount = metaObject->propertyCount();
  out["__propertyCount__"] = propertyCount;
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

void InspectorHandler::itemsPicked(const QList<QQuickItem*>& objects) {
  QStringList objectNames;
  for (QQuickItem* object : objects) {
    QString objectName = object->objectName();
    if (!objectName.isEmpty()) {
      objectNames.append(objectName);
    }
  }

  s_pickedItems = objectNames;
  s_pickedItemsSet = true;

  if (s_itemPicker) {
    qApp->removeEventFilter(s_itemPicker);
    s_itemPicker->deleteLater();
    s_itemPicker = nullptr;
  }
}
