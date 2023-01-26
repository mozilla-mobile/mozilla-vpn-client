/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhandler.h"

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
#include <functional>

#include "addons/manager/addonmanager.h"
#include "constants.h"
#include "controller.h"
#include "externalophandler.h"
#include "feature.h"
#include "frontend/navigator.h"
#include "inspectoritempicker.h"
#include "inspectorutils.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
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

#ifdef MZ_WASM
#  include "platforms/wasm/wasminspector.h"
#else
#  include <QCoreApplication>

#  include "inspectorwebsocketserver.h"
#endif

#ifdef MZ_ANDROID
#  include "platforms/android/androidvpnactivity.h"
#endif

namespace {
Logger logger("InspectorHandler");

bool s_forwardNetwork = false;
bool s_mockFreeTrial = false;

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

    InspectorCommand{"query", "Query the tree", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       obj["value"] =
                           !!InspectorUtils::queryObject(arguments[1]);
                       return obj;
                     }},

    InspectorCommand{
        "query_property", "Retrieve a property value from an object", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          QObject* item = InspectorUtils::queryObject(arguments[1]);
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

    InspectorCommand{
        "set_query_property", "Set a property value to an object", 3,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          QObject* item = InspectorUtils::queryObject(arguments[1]);
          if (!item) {
            obj["error"] = "Object not found";
            return obj;
          }

          const QMetaObject* metaObject = item->metaObject();
          int propertyId = metaObject->indexOfProperty(arguments[2]);
          if (propertyId < 0) {
            obj["error"] = "Invalid property";
            return obj;
          }

          QMetaProperty property = metaObject->property(propertyId);
          Q_ASSERT(property.isValid());

          QVariant value = QVariant::fromValue(arguments[3]);
          if (!value.canConvert(property.type())) {
            obj["error"] = "Property value is invalid";
            return obj;
          }

          property.write(item, value);
          return obj;
        }},

    InspectorCommand{
        "property", "Retrieve a property value from a Mozilla VPN object", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          int id = qmlTypeId("Mozilla.VPN", 1, 0, qPrintable(arguments[1]));

          QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
              QmlEngineHolder::instance()->engine());
          QObject* item = engine->singletonInstance<QObject*>(id);
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

    InspectorCommand{
        "set_property", "Set a property value to a Mozilla VPN object", 3,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          int id = qmlTypeId("Mozilla.VPN", 1, 0, qPrintable(arguments[1]));

          QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
              QmlEngineHolder::instance()->engine());
          QObject* item = engine->singletonInstance<QObject*>(id);
          if (!item) {
            obj["error"] = "Object not found";
            return obj;
          }

          const QMetaObject* metaObject = item->metaObject();
          int propertyId = metaObject->indexOfProperty(arguments[2]);
          if (propertyId < 0) {
            obj["error"] = "Invalid property";
            return obj;
          }

          QMetaProperty property = metaObject->property(propertyId);
          Q_ASSERT(property.isValid());

          QVariant value = QVariant::fromValue(arguments[3]);
          if (!value.canConvert(property.type())) {
            obj["error"] = "Property value is invalid";
            return obj;
          }

          property.write(item, value);
          return obj;
        }},

    InspectorCommand{"click", "Click on an object", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;

                       QObject* qmlobj =
                           InspectorUtils::queryObject(arguments[1]);
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

                       // It seems that in QT/QML there is a race-condition bug
                       // between the rendering thread and the main one when
                       // simulating clicks using QTest. At this point, all the
                       // properties are synchronized, the animation is
                       // probably already completed (otherwise it's a bug in
                       // the test!) but it could be that he following
                       // `mouse`Click` is ignored.
                       // The horrible/temporary solution is to wait a bit more
                       // and to add a delay (VPN-3697)
                       QTest::qWait(150);
                       QTest::mouseClick(item->window(), Qt::LeftButton,
                                         Qt::NoModifier, point);

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
          UrlOpener::instance()->setStealUrls();
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
                ->serverCountryModel()
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
        "set_setting", "Set a setting (setting, value)", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          SettingsHolder* settingsHolder = SettingsHolder::instance();
          const QMetaObject* metaObject = settingsHolder->metaObject();
          int propertyId = metaObject->indexOfProperty(arguments[1]);

          if (propertyId < 0) {
            obj["error"] =
                QString("Property %1 is not exposed by SettingsHolder")
                    .arg(QString(arguments[1]));
            return obj;
          }

          QMetaProperty property = metaObject->property(propertyId);
          Q_ASSERT(property.isValid());

          QVariant value = QVariant::fromValue(arguments[2]);
          if (value.canConvert(property.type())) {
            property.write(settingsHolder, value);
            return obj;
          }

          obj["error"] = QString("Property %1 has a non-supported type: %2")
                             .arg(arguments[1], property.type());
          return obj;
        }},

    InspectorCommand{
        "setting", "Get a setting value", 1,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          SettingsHolder* settingsHolder = SettingsHolder::instance();
          const QMetaObject* metaObject = settingsHolder->metaObject();
          int propertyId = metaObject->indexOfProperty(arguments[1]);

          if (propertyId < 0) {
            obj["error"] =
                QString("Property %1 is not exposed by SettingsHolder")
                    .arg(QString(arguments[1]));
            return obj;
          }

          QMetaProperty property = metaObject->property(propertyId);
          Q_ASSERT(property.isValid());

          QVariant prop_value = property.read(settingsHolder);
          if (prop_value.canConvert<QJsonValue>()) {
            obj["value"] = prop_value.toJsonValue();
            return obj;
          }

          if (prop_value.canConvert<QDateTime>()) {
            obj["value"] = prop_value.toDateTime().toString();
            return obj;
          }

          if (prop_value.canConvert<QByteArray>()) {
            obj["value"] = prop_value.toByteArray().data();
            return obj;
          }

          obj["error"] = QString("Property %1 has a non-supported type: %2")
                             .arg(arguments[1], property.type());
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
            for (const QString& cityName : country.cities()) {
              const ServerCity& city = scm->findCity(country.code(), cityName);
              if (!city.initialized()) {
                continue;
              }
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
#ifdef MZ_ANDROID
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

    InspectorCommand{"back_button_clicked",
                     "Simulate an android back-button clicked", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       Navigator::instance()->eventHandled();
                       return QJsonObject();
                     }},

    InspectorCommand{"copy_to_clipboard", "Copy text to clipboard", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString copiedText = arguments[1];
                       MozillaVPN::instance()->storeInClipboard(copiedText);
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
                       Localizer::instance()->forceRTL();
                       emit SettingsHolder::instance()->languageCodeChanged();
                       return QJsonObject();
                     }},

    InspectorCommand{"reset_addons",
                     "Reset all the addons cleaning up the cache", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       AddonManager::instance()->reset();
                       return QJsonObject();
                     }},

    InspectorCommand{"fetch_addons", "Force a fetch of the addon list manifest",
                     0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       AddonManager::instance()->fetch();
                       return QJsonObject();
                     }},
};

// static
void InspectorHandler::initialize() {
#ifdef MZ_WASM
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
  MZ_COUNT_CTOR(InspectorHandler);

  connect(LogHandler::instance(), &LogHandler::logEntryAdded, this,
          &InspectorHandler::logEntryAdded);

  connect(NotificationHandler::instance(),
          &NotificationHandler::notificationShown, this,
          &InspectorHandler::notificationShown);
  connect(NetworkManager::instance()->networkAccessManager(),
          &QNetworkAccessManager::finished, this,
          &InspectorHandler::networkRequestFinished);
  connect(AddonManager::instance(), &AddonManager::loadCompletedChanged, this,
          &InspectorHandler::addonLoadCompleted);
}

InspectorHandler::~InspectorHandler() { MZ_COUNT_DTOR(InspectorHandler); }

void InspectorHandler::recv(const QByteArray& command) {
  logger.debug() << "command received:" << command;

  if (command.isEmpty()) {
    return;
  }

  QList<QByteArray> parts = command.split(' ');
  Q_ASSERT(!parts.isEmpty());

  QString cmdName = parts[0].trimmed();
  for (int i = 1; i < parts.length(); ++i) {
    parts[i] = QUrl::fromPercentEncoding(parts[i]).toLocal8Bit();
  }

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

void InspectorHandler::addonLoadCompleted() {
  QJsonObject obj;
  obj["type"] = "addon_load_completed";
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
    return "Unknown";
  }
  auto metaObject = target->metaObject();
  return metaObject->className();
}

// static
bool InspectorHandler::mockFreeTrial() { return s_mockFreeTrial; }

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
