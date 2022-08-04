/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addon.h"
#include "addondemo.h"
#include "addonguide.h"
#include "addoni18n.h"
#include "addonmessage.h"
#include "addontutorial.h"
#include "conditionwatchers/addonconditionwatchergroup.h"
#include "conditionwatchers/addonconditionwatcherlocales.h"
#include "conditionwatchers/addonconditionwatchertriggertimesecs.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "mozillavpn.h"
#include "settingsholder.h"
#include "update/versionapi.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
Logger logger(LOG_MAIN, "Addon");

bool evaluateConditionsSettingsOp(const QString& op, bool result) {
  if (op == "eq") return result;

  if (op == "neq") return !result;

  logger.warning() << "Invalid settings operator" << op;
  return false;
}

struct ConditionCallback {
  QString m_key;
  std::function<bool(const QJsonValue&)> m_staticCallback;
  std::function<AddonConditionWatcher*(QObject*, const QJsonValue&)>
      m_dynamicCallback;
};

QList<ConditionCallback> s_conditionCallbacks{
    {"enabled_features",
     [](const QJsonValue& value) -> bool {
       for (const QJsonValue& enabledFeature : value.toArray()) {
         QString featureName = enabledFeature.toString();

         // If the feature doesn't exist, we crash.
         const Feature* feature = Feature::get(featureName);
         if (!feature) {
           logger.info() << "Feature not found" << featureName;
           return false;
         }

         if (!feature->isSupported()) {
           logger.info() << "Feature not supported" << featureName;
           return false;
         }
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"platforms",
     [](const QJsonValue& value) -> bool {
       QStringList platforms;
       for (QJsonValue platform : value.toArray()) {
         platforms.append(platform.toString());
       }

       if (!platforms.isEmpty() &&
           !platforms.contains(MozillaVPN::instance()->platform())) {
         logger.info() << "Not supported platform";
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"settings",
     [](const QJsonValue& value) -> bool {
       for (QJsonValue setting : value.toArray()) {
         QJsonObject obj = setting.toObject();

         QString op = obj["op"].toString();
         QString key = obj["setting"].toString();
         QVariant valueA = SettingsHolder::instance()->rawSetting(key);
         if (!valueA.isValid()) {
           logger.info() << "Unable to retrieve setting key" << key;
           return false;
         }

         QJsonValue valueB = obj["value"];
         switch (valueB.type()) {
           case QJsonValue::Bool:
             if (!evaluateConditionsSettingsOp(
                     op, valueA.toBool() == valueB.toBool())) {
               logger.info() << "Setting value doesn't match for key" << key;
               return false;
             }

             break;

           case QJsonValue::Double:
             if (!evaluateConditionsSettingsOp(
                     op, valueA.toDouble() == valueB.toDouble())) {
               logger.info() << "Setting value doesn't match for key" << key;
               return false;
             }

             break;
             break;

           case QJsonValue::String:
             if (!evaluateConditionsSettingsOp(
                     op, valueA.toString() == valueB.toString())) {
               logger.info() << "Setting value doesn't match for key" << key;
               return false;
             }

             break;

           default:
             logger.warning()
                 << "Unsupported setting value type for key" << key;
             return false;
         }
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"env",
     [](const QJsonValue& value) -> bool {
       QString env = value.toString();

       if (env.isEmpty()) {
         return true;
       }

       if (env == "staging") {
         return !Constants::inProduction();
       }

       if (env == "production") {
         return Constants::inProduction();
       }

       logger.info() << "Unknown env value:" << env;
       return false;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"min_client_version",
     [](const QJsonValue& value) -> bool {
       QString min = value.toString();
       QString currentVersion = Constants::versionString();

       if (!min.isEmpty() &&
           VersionApi::compareVersions(min, currentVersion) == 1) {
         logger.info() << "Min version is" << min << " curent"
                       << currentVersion;
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"max_client_version",
     [](const QJsonValue& value) -> bool {
       QString max = value.toString();
       QString currentVersion = Constants::versionString();

       if (!max.isEmpty() &&
           VersionApi::compareVersions(max, currentVersion) == -1) {
         logger.info() << "Max version is" << max << " curent"
                       << currentVersion;
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     }},

    {"locales",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](QObject* parent, const QJsonValue& value) -> AddonConditionWatcher* {
       QStringList locales;
       for (const QJsonValue& v : value.toArray()) {
         locales.append(v.toString().toLower());
       }

       return AddonConditionWatcherLocales::maybeCreate(parent, locales);
     }},

    {"trigger_time",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](QObject* parent, const QJsonValue& value) -> AddonConditionWatcher* {
       return AddonConditionWatcherTriggerTimeSecs::maybeCreate(
           parent, value.toInteger());
     }},
};

}  // namespace

// static
Addon* Addon::create(QObject* parent, const QString& manifestFileName) {
  QFile file(manifestFileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the addon manifest of"
                     << manifestFileName;
    return nullptr;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isObject()) {
    logger.warning() << "The manifest must be a JSON document"
                     << manifestFileName;
    return nullptr;
  }

  QJsonObject obj = json.object();

  QString version = obj["api_version"].toString();
  if (version.isEmpty()) {
    logger.warning() << "No API version in the manifest" << manifestFileName;
    return nullptr;
  }

  if (version != "0.1") {
    logger.warning() << "Unsupported API version" << version
                     << manifestFileName;
    return nullptr;
  }

  QJsonObject conditions = obj["conditions"].toObject();
  if (!evaluateConditions(conditions)) {
    logger.info() << "Exclude the addon because conditions do not match"
                  << manifestFileName;
    return nullptr;
  }

  QString id = obj["id"].toString();
  if (id.isEmpty()) {
    logger.warning() << "No id in the manifest" << manifestFileName;
    return nullptr;
  }

  QString name = obj["name"].toString();
  if (name.isEmpty()) {
    logger.warning() << "No name in the manifest" << manifestFileName;
    return nullptr;
  }

  QString type = obj["type"].toString();
  if (type.isEmpty()) {
    logger.warning() << "No type in the manifest" << manifestFileName;
    return nullptr;
  }

  Addon* addon = nullptr;

  if (!Constants::inProduction() && type == "demo") {
    addon = AddonDemo::create(parent, manifestFileName, id, name, obj);
  }

  else if (type == "i18n") {
    addon = new AddonI18n(parent, manifestFileName, id, name);
  }

  else if (type == "tutorial") {
    addon = AddonTutorial::create(parent, manifestFileName, id, name, obj);
  }

  else if (type == "guide") {
    addon = AddonGuide::create(parent, manifestFileName, id, name, obj);
  }

  else if (type == "message") {
    addon = AddonMessage::create(parent, manifestFileName, id, name, obj);
  }

  else {
    logger.warning() << "Unsupported type" << type << manifestFileName;
    return nullptr;
  }

  if (!addon) {
    return nullptr;
  }

  addon->maybeCreateConditionWatchers(conditions);

  if (addon->enabled()) {
    addon->enable();
  }

  return addon;
}

Addon::Addon(QObject* parent, const QString& manifestFileName,
             const QString& id, const QString& name, const QString& type)
    : QObject(parent),
      m_manifestFileName(manifestFileName),
      m_id(id),
      m_name(name),
      m_type(type) {
  MVPN_COUNT_CTOR(Addon);
}

Addon::~Addon() {
  MVPN_COUNT_DTOR(Addon);
  disable();
}

void Addon::retranslate() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString code = settingsHolder->languageCode();

  QLocale locale = QLocale(code);
  if (code.isEmpty()) {
    locale = QLocale(QLocale::system().bcp47Name());
  }

  if (!m_translator.load(
          locale, "locale", "_",
          QFileInfo(m_manifestFileName).dir().filePath("i18n"))) {
    logger.error() << "Loading the locale failed. - code:" << code;
  }
}

void Addon::maybeCreateConditionWatchers(const QJsonObject& conditions) {
  QList<AddonConditionWatcher*> watcherList;

  for (const QString& key : conditions.keys()) {
    for (const ConditionCallback& condition : s_conditionCallbacks) {
      if (condition.m_key == key) {
        AddonConditionWatcher* conditionWatcher =
            condition.m_dynamicCallback(this, conditions[key]);
        if (conditionWatcher) {
          watcherList.append(conditionWatcher);
        }
        break;
      }
    }
  }

  switch (watcherList.length()) {
    case 0:
      return;

    case 1:
      m_conditionWatcher = watcherList.at(0);
      break;

    default:
      m_conditionWatcher = new AddonConditionWatcherGroup(this, watcherList);
      break;
  }

  Q_ASSERT(m_conditionWatcher);

  connect(m_conditionWatcher, &AddonConditionWatcher::conditionChanged, this,
          [this](bool enabled) {
            if (enabled) {
              enable();
            } else {
              disable();
            }
          });
}

// static
bool Addon::evaluateConditions(const QJsonObject& conditions) {
  for (const QString& key : conditions.keys()) {
    bool found = false;
    for (const ConditionCallback& condition : s_conditionCallbacks) {
      if (condition.m_key == key) {
        if (!condition.m_staticCallback(conditions[key])) {
          return false;
        }
        found = true;
        break;
      }
    }

    if (!found) {
      logger.error() << "Invalid condition key" << key;
      return false;
    }
  }

  return true;
}

bool Addon::enabled() const {
  if (!m_conditionWatcher) {
    return true;
  }

  return m_conditionWatcher->conditionApplied();
}

void Addon::enable() {
  QCoreApplication::installTranslator(&m_translator);
  retranslate();

  emit conditionChanged(true);
}

void Addon::disable() {
  QCoreApplication::removeTranslator(&m_translator);

  emit conditionChanged(false);
}
