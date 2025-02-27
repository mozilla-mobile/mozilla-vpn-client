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

#if QT_CONFIG(thread)
#  include <QRunnable>
#  include <QSemaphore>
#endif

#include "constants.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "inspectorhotreloader.h"
#include "inspectoritempicker.h"
#include "inspectorutils.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "loghandler.h"
#include "mzglean.h"
#include "networkmanager.h"
#include "qmlengineholder.h"
#include "settings/settingsmanager.h"
#include "settingsholder.h"
#include "task.h"
#include "urlopener.h"
#include "utils.h"

#ifdef MZ_WASM
#  include "wasminspector.h"
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
          if (!value.canConvert(property.metaType())) {
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
          if (!value.canConvert(property.metaType())) {
            obj["error"] = "Property value is invalid";
            return obj;
          }

          property.write(item, value);
          return obj;
        }},

    InspectorCommand{"click", "Click on an object", 1,
                     [](InspectorHandler*, const QList<QByteArray>& arguments) {
                       QJsonObject obj;
                       QQuickWindow* window = qobject_cast<QQuickWindow*>(
                           QmlEngineHolder::instance()->window());

#if QT_CONFIG(thread)
                       // If Qt is multithreaded, we may need to ensure that
                       // window rendering is finished in order to make sure
                       // that the QSceneGraph is sychronized with what's on
                       // screen.
                       QSemaphore sem(0);
                       QRunnable* job =
                           QRunnable::create([&sem] { sem.release(); });

                       window->update();
                       window->scheduleRenderJob(job, QQuickWindow::NoStage);
                       if (!sem.tryAcquire(1, 150)) {
                         logger.warning() << "Window rendering never finished";
                       }
#endif

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

                       QPointF center = item->boundingRect().center();
                       QTest::mouseClick(window, Qt::LeftButton, Qt::NoModifier,
                                         item->mapToScene(center).toPoint());
                       return obj;
                     }},

    InspectorCommand{
        "scrollview", "Scroll a view until an item is centered", 2,
        [](InspectorHandler*, const QList<QByteArray>& arguments) {
          QJsonObject result;

          QObject* viewobj = InspectorUtils::queryObject(arguments[1]);
          if (!viewobj) {
            logger.error() << "Did not find view object";
            result["error"] = "Object not found";
            return result;
          }
          QQuickItem* view = qobject_cast<QQuickItem*>(viewobj);
          if (!view) {
            logger.error() << "View is not a QQuickItem object";
            result["error"] = "Object not found";
            return result;
          }
          const QMetaObject* viewMeta = view->metaObject();
          int scrollPropId = viewMeta->indexOfProperty("contentY");
          if (scrollPropId < 0) {
            logger.error() << "View is not scrollable";
            result["error"] = "View is not scrollable";
            return result;
          }

          QObject* targetobj = InspectorUtils::queryObject(arguments[2]);
          if (!targetobj) {
            logger.error() << "Did not find target object";
            result["error"] = "Object not found";
            return result;
          }
          QQuickItem* target = qobject_cast<QQuickItem*>(targetobj);
          if (!target) {
            logger.error() << "Target is not a QQuickItem object";
            result["error"] = "Object not found";
            return result;
          }

          // Get the size of the view and calculate scroll limits.
          qreal current = view->property("contentY").toReal();
          qreal content = view->property("contentHeight").toReal();
          qreal height = view->property("height").toReal();
          qreal limit = (content > height) ? content - height : 0;

          // Get the vertical position, in the view's coordinates.
          QPointF center = target->boundingRect().center();
          qreal offset = target->mapToItem(view, center).y();

          // Figure out how much to scroll to center the target.
          qreal contentY = current + offset - (height / 2);
          if (contentY < 0) contentY = 0;
          if (contentY > limit) contentY = limit;

          // Scroll the view.
          QMetaProperty prop = viewMeta->property(scrollPropId);
          Q_ASSERT(prop.isValid());
          if (!prop.write(view, QVariant::fromValue(contentY))) {
            logger.error() << "Could not write property contentY";
            result["error"] = "Could not write property contentY";
            return result;
          }

          return result;
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
          if (value.canConvert(property.metaType())) {
            property.write(settingsHolder, value);
            return obj;
          }

          obj["error"] = QString("Property %1 has a non-supported type: %2")
                             .arg(arguments[1], property.typeName());
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
                             .arg(arguments[1], property.typeName());
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
          QString ping = QString(arguments[3]);
          QJsonObject obj;

          QObject* instance = __DONOTUSE__GleanMetrics::instance();
          QVariant qvCategory = instance->property(arguments[1].constData());
          QObject* category = qvCategory.value<QObject*>();
          if (category == nullptr) {
            obj["error"] = QString(arguments[1]) + "is not a valid category";
            return obj;
          }

          QVariant metric = category->property(arguments[2].constData());
          if (!metric.isValid()) {
            obj["error"] = QString(arguments[2]) + "is not a valid metric";
            return obj;
          }

          if (metric.canConvert<BaseMetric*>()) {
            BaseMetric* baseMetric = metric.value<BaseMetric*>();
            obj["value"] = baseMetric->testGetValue(ping);
            return obj;
          }

          QString className = "unknown";
          if (metric.canConvert<QObject*>()) {
            className = metric.value<QObject*>()->metaObject()->className();
          }
          obj["error"] =
              QString("not a supported metric type (%1)").arg(className);
          return obj;
        }},

    InspectorCommand{"glean_test_reset", "Resets Glean for testing", 0,
                     [](InspectorHandler*, const QList<QByteArray>&) {
                       MZGlean::initialize("testing");
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
      if (obj.contains("error")) {
        logger.debug() << "command failed:" << obj["error"].toString();
      }
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
