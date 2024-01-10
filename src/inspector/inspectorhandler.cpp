/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhandler.h"

#include <QBuffer>
#include <QJSValue>
#include <QJSValueIterator>
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

#include "context/constants.h"
#include "context/qmlengineholder.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "inspectorhotreloader.h"
#include "inspectoritempicker.h"
#include "inspectorutils.h"
#include "localizer.h"
#include "logging/logger.h"
#include "logging/loghandler.h"
#include "models/subscriptiondata.h"
#include "mozillavpn.h"
#include "mzglean.h"
#include "networkmanager.h"
#include "settings/settingsholder.h"
#include "settings/settingsmanager.h"
#include "task.h"
#include "utilities/leakdetector.h"
#include "utilities/urlopener.h"
#include "utilities/utils.h"

#ifdef MZ_WASM
#  include "platforms/wasm/wasminspector.h"
#else
#  include <QCoreApplication>

#  include "inspectorwebsocketserver.h"
#endif

namespace {
Logger logger("InspectorHandler");

bool s_forwardNetwork = false;

QStringList s_pickedItems;
bool s_pickedItemsSet = false;

InspectorItemPicker* s_itemPicker = nullptr;

std::function<void(InspectorHandler* inspectorHandler)> s_constructorCallback;
InspectorHotreloader* s_hotreloader = nullptr;

QJsonValue qjsValueToQJsonValue(const QJSValue& qjsValue) {
  if (qjsValue.isUndefined() || qjsValue.isNull()) {
    return QJsonValue(QJsonValue::Undefined);
  } else if (qjsValue.isBool()) {
    return QJsonValue(qjsValue.toBool());
  } else if (qjsValue.isNumber()) {
    return QJsonValue(qjsValue.toNumber());
  } else if (qjsValue.isString()) {
    return QJsonValue(qjsValue.toString());
  } else if (qjsValue.isArray()) {
    QJsonArray jsonArray;
    quint32 length = qjsValue.property("length").toUInt();
    for (quint32 i = 0; i < length; ++i) {
      jsonArray.append(qjsValueToQJsonValue(qjsValue.property(i)));
    }
    return QJsonValue(jsonArray);
  } else if (qjsValue.isObject()) {
    QJsonObject jsonObject;
    QJSValueIterator it(qjsValue);
    while (it.hasNext()) {
      it.next();
      jsonObject.insert(it.name(), qjsValueToQJsonValue(it.value()));
    }
    return QJsonValue(jsonObject);
  }
  // Handle other types as needed
  return QJsonValue(QJsonValue::Undefined);
}
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

    InspectorCommand{"fetch_network", "Enables forwarding of networkRequests",
                     0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       s_forwardNetwork = true;
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

    InspectorCommand{"view_tree", "Sends a view tree", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       return InspectorHandler::getViewTree();
                     }},

    InspectorCommand{"live_reload", "Live reload file X", 1,
                     [](InspectorHandler*, const QList<QByteArray>& args) {
                       if (s_hotreloader == nullptr) {
                         s_hotreloader = new InspectorHotreloader(
                             QmlEngineHolder::instance()->engine());
                       }
                       auto url = QUrl(args.at(1));
                       s_hotreloader->annonceReplacedFile(url);
                       return QJsonObject();
                     }},
    InspectorCommand{"reload_window", "Reload the whole window", 0,
                     [](InspectorHandler*, const QList<QByteArray>& args) {
                       if (s_hotreloader == nullptr) {
                         s_hotreloader = new InspectorHotreloader(
                             QmlEngineHolder::instance()->engine());
                       }
                       s_hotreloader->reloadWindow();
                       return QJsonObject();
                     }},
    InspectorCommand{"reset_live_reload", "Reset all hot reloaded files", 0,
                     [](InspectorHandler*, const QList<QByteArray>& args) {
                       if (s_hotreloader == nullptr) {
                         return QJsonObject();
                       }
                       s_hotreloader->resetAllFiles();
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
        "property", "Retrieve a property value from a Mozilla object", 3,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          int id = qmlTypeId(qPrintable(arguments[1]), 1, 0,
                             qPrintable(arguments[2]));

          QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
              QmlEngineHolder::instance()->engine());
          QObject* item = engine->singletonInstance<QObject*>(id);
          if (!item) {
            obj["error"] = "Object not found";
            return obj;
          }

          QVariant property = item->property(arguments[3]);
          if (!property.isValid()) {
            obj["error"] = "Property is invalid";
            return obj;
          }

          obj["value"] = property.toString();
          return obj;
        }},

    InspectorCommand{
        "set_property", "Set a property value to a Mozilla object", 4,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject obj;

          int id = qmlTypeId(qPrintable(arguments[1]), 1, 0,
                             qPrintable(arguments[2]));

          QQmlApplicationEngine* engine = qobject_cast<QQmlApplicationEngine*>(
              QmlEngineHolder::instance()->engine());
          QObject* item = engine->singletonInstance<QObject*>(id);
          if (!item) {
            obj["error"] = "Object not found";
            return obj;
          }

          const QMetaObject* metaObject = item->metaObject();
          int propertyId = metaObject->indexOfProperty(arguments[3]);
          if (propertyId < 0) {
            obj["error"] = "Invalid property";
            return obj;
          }

          QMetaProperty property = metaObject->property(propertyId);
          Q_ASSERT(property.isValid());

          QVariant value = QVariant::fromValue(arguments[4]);
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

                       QPointF pointF = item->mapToScene(QPoint(0, 0));
                       QPoint point = pointF.toPoint();

                       QTest::mouseClick(item->window(), Qt::LeftButton,
                                         Qt::NoModifier, point);

                       return obj;
                     }},

    InspectorCommand{
        "stealurls",
        "Do not open the URLs in browser and expose them via inspector", 0,
        [](InspectorHandler*, const QList<QByteArray>&) {
          UrlOpener::instance()->setStealUrls();
          return QJsonObject();
        }},

    InspectorCommand{"lasturl", "Retrieve the last opened URL", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       QJsonObject obj;
                       obj["value"] = UrlOpener::instance()->lastUrl();
                       return obj;
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
                           SettingsManager::instance()->settingsFileName();
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

    // Differs from "is_feature_flipped_on" because here we don't care if the
    // feature has or has not been manually toggled AKA "flipped"  (via UI,
    // inspector, etc) we just care if it is enabled or not
    InspectorCommand{"is_feature_enabled", "Check if a feature is enabled", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       QString featureName = arguments[1];
                       const Feature* feature = Feature::getOrNull(featureName);

                       if (!feature) {
                         obj["error"] = "Feature does not exist";
                         return obj;
                       }

                       bool featureEnabled = feature->isSupported();

                       obj["value"] = featureEnabled;
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

    InspectorCommand{"back_button_clicked",
                     "Simulate an android back-button clicked", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       Navigator::instance()->eventHandled();
                       return QJsonObject();
                     }},

    InspectorCommand{"copy_to_clipboard", "Copy text to clipboard", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString copiedText = arguments[1];
                       Utils::storeInClipboard(copiedText);
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

    InspectorCommand{"set_version_override", "Override the version string", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QString versionOverride = QString(arguments[1]);
                       Constants::setVersionOverride(versionOverride);
                       return QJsonObject();
                     }},

    InspectorCommand{
        "glean_test_get_value", "Get value of a Glean metric", 3,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QString metricCategory = QString(arguments[1]);
          QString metricName = QString(arguments[2]);
          QString ping = QString(arguments[3]);

          // Hack: let's run the code on the QML side,
          // because I (Bea) could not figure out a nice way
          // to access C++ namespaces and namespace properties
          // with runtime strings.
          QJSEngine* engine = QmlEngineHolder::instance()->engine();
          QJSValue function = QmlEngineHolder::instance()->engine()->evaluate(
              QString("(Glean) => Glean.%1.%2.testGetValue(\"%3\")")
                  .arg(metricCategory)
                  .arg(metricName)
                  .arg(ping));

          QJSValue result = function.call(QJSValueList{
              engine->toScriptValue(__DONOTUSE__GleanMetrics::instance())});

          QJsonObject obj;
          if (result.isError()) {
            obj["error"] = result.toString();
          } else {
            obj["value"] = qjsValueToQJsonValue(result);
          }

          return obj;
        }},

    InspectorCommand{"glean_test_reset", "Resets Glean for testing", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MZGlean::initialize();
                       return QJsonObject();
                     }},

    InspectorCommand{
        "set_subscription_start_date",
        "Changes the start date of the subscription", 1,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          qint64 newCreatedAtTimestamp = arguments[1].toLongLong();

          // get sub data json from settings
          QByteArray subscriptionData =
              SettingsHolder::instance()->subscriptionData();
          QJsonDocument doc = QJsonDocument::fromJson(subscriptionData);
          QJsonObject obj = doc.object();
          QJsonObject subObj = obj["subscription"].toObject();

          // modify createdAt date
          qlonglong createdAt = newCreatedAtTimestamp;
          subObj["created"] = createdAt;
          obj["subscription"] = subObj;
          doc.setObject(obj);
          subscriptionData = doc.toJson();
          SettingsHolder::instance()->setSubscriptionData(subscriptionData);

          return QJsonObject();
        }}};

// static
void InspectorHandler::initialize() {
#ifdef MZ_WASM
  WasmInspector::instance();
#else
  if (!Constants::inProduction()) {
    InspectorWebSocketServer* inspectWebSocketServer =
        new InspectorWebSocketServer(qApp);
    QObject::connect(qApp, &QCoreApplication::aboutToQuit,
                     inspectWebSocketServer, &InspectorWebSocketServer::close);
  }
#endif
}

InspectorHandler::InspectorHandler(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(InspectorHandler);

  connect(LogHandler::instance(), &LogHandler::logEntryAdded, this,
          &InspectorHandler::logEntryAdded);

  connect(NetworkManager::instance()->networkAccessManager(),
          &QNetworkAccessManager::finished, this,
          &InspectorHandler::networkRequestFinished);

  if (s_constructorCallback) {
    s_constructorCallback(this);
  }
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

// static
void InspectorHandler::registerCommand(
    const QString& commandName, const QString& commandDescription,
    int32_t arguments,
    std::function<QJsonObject(InspectorHandler*, const QList<QByteArray>&)>&&
        callback) {
  s_commands.append(InspectorCommand{commandName, commandDescription, arguments,
                                     std::move(callback)});
}

// static
void InspectorHandler::setConstructorCallback(
    std::function<void(InspectorHandler* inspectorHandler)>&& callback) {
  s_constructorCallback = std::move(callback);
}
