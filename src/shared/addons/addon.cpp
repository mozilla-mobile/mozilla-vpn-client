/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addon.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "addonapi.h"
#include "addonguide.h"
#include "addoni18n.h"
#include "addonmessage.h"
#include "addonreplacer.h"
#include "addontutorial.h"
#include "conditionwatchers/addonconditionwatcherfeaturesenabled.h"
#include "conditionwatchers/addonconditionwatchergroup.h"
#include "conditionwatchers/addonconditionwatcherjavascript.h"
#include "conditionwatchers/addonconditionwatcherlocales.h"
#include "conditionwatchers/addonconditionwatchertimeend.h"
#include "conditionwatchers/addonconditionwatchertimestart.h"
#include "conditionwatchers/addonconditionwatchertranslationthreshold.h"
#include "conditionwatchers/addonconditionwatchertriggertimesecs.h"
#include "env.h"
#include "feature.h"
#include "glean/generated/metrics.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "state/addonsessionstate.h"
#include "versionutils.h"

namespace {
Logger logger("Addon");

bool evaluateConditionsSettingsOp(const QString& op, bool result) {
  if (op == "eq") return result;

  if (op == "neq") return !result;

  logger.warning() << "Invalid settings operator" << op;
  return false;
}

struct ConditionCallback {
  QString m_key;
  std::function<bool(const QJsonValue&)> m_staticCallback;
  std::function<AddonConditionWatcher*(Addon*, const QJsonValue&)>
      m_dynamicCallback;
  std::function<AddonConditionWatcher*(Addon*)> m_defaultCallback;
};

QList<ConditionCallback> s_conditionCallbacks{
    {"enabled_features",
     [](const QJsonValue& value) -> bool {
       const QJsonArray enabledFeatures = value.toArray();
       for (const QJsonValue& enabledFeature : enabledFeatures) {
         QString featureName = enabledFeature.toString();

         const Feature* feature = Feature::getOrNull(featureName);
         if (!feature) {
           logger.info() << "Feature not found" << featureName;
           return false;
         }
       }

       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       QStringList features;
       QJsonArray featureArray = value.toArray();
       for (const QJsonValue& v : featureArray) {
         features.append(v.toString());
       }

       return AddonConditionWatcherFeaturesEnabled::maybeCreate(addon,
                                                                features);
     },
     nullptr},

    {"platforms",
     [](const QJsonValue& value) -> bool {
       QStringList platforms;
       QJsonArray platformArray = value.toArray();
       for (const QJsonValue& platform : platformArray) {
         platforms.append(platform.toString());
       }

       if (!platforms.isEmpty() && !platforms.contains(Env::platform())) {
         logger.info() << "Not supported platform";
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     },
     nullptr},

    {"settings",
     [](const QJsonValue& value) -> bool {
       QJsonArray settings = value.toArray();
       for (const QJsonValue& setting : settings) {
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
     },
     nullptr},

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
     },
     nullptr},

    {"min_client_version",
     [](const QJsonValue& value) -> bool {
       QString min = value.toString();
       QString currentVersion = Constants::versionString();

       if (!min.isEmpty() &&
           VersionUtils::compareVersions(min, currentVersion) == 1) {
         logger.info() << "Min version is" << min << " curent"
                       << currentVersion;
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     },
     nullptr},

    {"max_client_version",
     [](const QJsonValue& value) -> bool {
       QString max = value.toString();
       QString currentVersion = Constants::versionString();

       if (!max.isEmpty() &&
           VersionUtils::compareVersions(max, currentVersion) == -1) {
         logger.info() << "Max version is" << max << " curent"
                       << currentVersion;
         return false;
       }

       return true;
     },
     [](QObject*, const QJsonValue&) -> AddonConditionWatcher* {
       return nullptr;
     },
     nullptr},

    {"locales",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       QStringList locales;
       QJsonArray localeArray = value.toArray();
       for (const QJsonValue& v : localeArray) {
         locales.append(v.toString().toLower());
       }

       return AddonConditionWatcherLocales::maybeCreate(addon, locales);
     },
     nullptr},

    {"trigger_time",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       return AddonConditionWatcherTriggerTimeSecs::maybeCreate(
           addon, value.toInteger());
     },
     nullptr},

    {"start_time",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       return new AddonConditionWatcherTimeStart(addon, value.toInteger());
     },
     nullptr},

    {"end_time",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       return new AddonConditionWatcherTimeEnd(addon, value.toInteger());
     },
     nullptr},

    {"javascript",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       return AddonConditionWatcherJavascript::maybeCreate(addon,
                                                           value.toString());
     },
     nullptr},

    {"translation_threshold",
     [](const QJsonValue&) -> bool {
       // dynamic condition
       return true;
     },
     [](Addon* addon, const QJsonValue& value) -> AddonConditionWatcher* {
       return AddonConditionWatcherTranslationThreshold::maybeCreate(
           addon, value.toDouble());
     },
     [](Addon* addon) -> AddonConditionWatcher* {
       return AddonConditionWatcherTranslationThreshold::maybeCreate(addon,
                                                                     1.0);
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

  if (type == "i18n") {
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

  else if (type == "replacer") {
    addon = AddonReplacer::create(parent, manifestFileName, id, name, obj);
  }

  else {
    logger.warning() << "Unsupported type" << type << manifestFileName;
    return nullptr;
  }

  if (!addon) {
    return nullptr;
  }

  addon->m_state = new AddonState(addon, obj["state"].toObject());

  QJsonObject javascript = obj["javascript"].toObject();
  if (!addon->evaluateJavascript(javascript)) {
    addon->deleteLater();
    return nullptr;
  }

  addon->m_conditionWatcher =
      Addon::maybeCreateConditionWatchers(addon, conditions);

  if (addon->m_conditionWatcher) {
    connect(addon->m_conditionWatcher, &AddonConditionWatcher::conditionChanged,
            addon, [addon](bool enabled) {
              if (enabled != addon->m_enabled) {
                if (enabled) {
                  addon->enable();
                } else {
                  addon->disable();
                }
              }
            });
  }

  if (!addon->m_conditionWatcher ||
      addon->m_conditionWatcher->conditionApplied()) {
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
  MZ_COUNT_CTOR(Addon);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString statusSetting = settingsHolder->getAddonSetting(StatusQuery(id));
  QMetaEnum statusMetaEnum = QMetaEnum::fromType<Status>();

  bool isValidStatus = false;
  int persistedStatus = statusMetaEnum.keyToValue(
      statusSetting.toLocal8Bit().constData(), &isValidStatus);

  if (isValidStatus) {
    m_status = static_cast<Status>(persistedStatus);
  }

  if (m_status == Unknown) {
    updateAddonStatus(Installed);
  }

  QFileInfo manifestFileInfo(manifestFileName);
  QDir addonPath = manifestFileInfo.dir();
  if (addonPath.cd("i18n") && addonPath.exists("translations.completeness")) {
    m_translationCompleteness = Localizer::loadTranslationCompleteness(
        addonPath.filePath("translations.completeness"));
  }
}

Addon::~Addon() { MZ_COUNT_DTOR(Addon); }

void Addon::updateAddonStatus(Status newStatus) {
  Q_ASSERT(newStatus != Unknown);

  if (m_status == newStatus) {
    return;
  }

  m_status = newStatus;

  QMetaEnum statusMetaEnum = QMetaEnum::fromType<Status>();
  QString newStatusSetting = statusMetaEnum.valueToKey(newStatus);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  settingsHolder->setAddonSetting(StatusQuery(id()), newStatusSetting);

  mozilla::glean::sample::addon_state_changed.record(
      mozilla::glean::sample::AddonStateChangedExtra{
          ._addonId = m_id,
          ._state = newStatusSetting,
      });
}

void Addon::retranslate() {
  unloadTranslators();

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QString localeCode = Localizer::instance()->languageCodeOrSystem();

  double completeness = m_translationCompleteness.value(localeCode, 0);
  if (completeness < 1) {
    logger.debug() << "Let's try to load another language as fallback for code"
                   << localeCode;
    maybeLoadLanguageFallback(localeCode);
  }

  QLocale locale = Localizer::instance()->locale();

  if (!createTranslator(locale)) {
    logger.error() << "Loading the locale failed - code:" << localeCode;
  }

  emit retranslationCompleted();
}

AddonConditionWatcher* Addon::maybeCreateConditionWatchers(
    Addon* addon, const QJsonObject& conditions) {
  QList<AddonConditionWatcher*> watcherList;

  for (const ConditionCallback& condition : s_conditionCallbacks) {
    if (conditions.contains(condition.m_key)) {
      AddonConditionWatcher* conditionWatcher =
          condition.m_dynamicCallback(addon, conditions[condition.m_key]);
      if (conditionWatcher) {
        watcherList.append(conditionWatcher);
      }

      continue;
    }

    if (condition.m_defaultCallback) {
      AddonConditionWatcher* conditionWatcher =
          condition.m_defaultCallback(addon);
      if (conditionWatcher) {
        watcherList.append(conditionWatcher);
      }
    }
  }

  switch (watcherList.length()) {
    case 0:
      return nullptr;

    case 1:
      return watcherList.at(0);

    default:
      return new AddonConditionWatcherGroup(addon, watcherList);
  }
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

void Addon::enable() {
  m_enabled = true;

  retranslate();

  if (m_jsEnableFunction.isCallable()) {
    QJSEngine* engine = QmlEngineHolder::instance()->engine();
    AddonApi* apiObj = api();
    QJSValue api = engine->newQObject(apiObj);

    QJSValue output = m_jsEnableFunction.call(QJSValueList{api});
    if (output.isError()) {
      logger.debug() << "Execution of enable javascript function failed"
                     << output.toString();
    }
  }

  updateAddonStatus(Status::Enabled);
  emit conditionChanged(true);
}

void Addon::disable() {
  emit aboutToDisable();
  m_enabled = false;

  unloadTranslators();

  if (m_jsDisableFunction.isCallable()) {
    QJSEngine* engine = QmlEngineHolder::instance()->engine();
    AddonApi* apiObj = api();
    QJSValue api = engine->newQObject(apiObj);

    QJSValue output = m_jsDisableFunction.call(QJSValueList{api});
    if (output.isError()) {
      logger.debug() << "Execution of disable javascript function failed"
                     << output.toString();
    }
  }

  updateAddonStatus(Status::Disabled);
  emit conditionChanged(false);
}

AddonApi* Addon::api() {
  if (!m_api) {
    m_api = new AddonApi(this);
  }

  return m_api;
}

bool Addon::evaluateJavascript(const QJsonObject& javascript) {
  return evaluateJavascriptInternal(javascript["enable"].toString(),
                                    &m_jsEnableFunction) &&
         evaluateJavascriptInternal(javascript["disable"].toString(),
                                    &m_jsDisableFunction);
}

bool Addon::evaluateJavascriptInternal(const QString& javascript,
                                       QJSValue* value) {
  if (javascript.isEmpty()) {
    // Not an error.
    return true;
  }

  QFileInfo manifestFileInfo(manifestFileName());
  QDir addonPath = manifestFileInfo.dir();

  QFile file(addonPath.filePath(javascript));
  if (!file.open(QIODevice::ReadOnly)) {
    logger.debug() << "Unable to open the javascript file" << javascript;
    return false;
  }

  QJSValue output =
      QmlEngineHolder::instance()->engine()->evaluate(file.readAll());
  if (output.isError()) {
    logger.debug() << "Execution throws an error:" << output.toString();
    return false;
  }

  if (!output.isCallable()) {
    logger.debug() << "The javascript entry should be a callable function";
    return false;
  }

  *value = output;
  return true;
}

void Addon::unloadTranslators() {
  for (QTranslator* translator : m_translators) {
    QCoreApplication::removeTranslator(translator);
    translator->deleteLater();
  }
  m_translators.clear();
}

void Addon::maybeLoadLanguageFallback(const QString& code) {
  // First fallback, English where we are 100% sure we have all the
  // translations. If something goes totally wrong, we use English strings.
  if (!createTranslator(QLocale("en"))) {
    logger.warning() << "Loading the fallback locale failed - code: en";
  }

  for (const QString& fallbackCode :
       Localizer::instance()->fallbackForLanguage(code)) {
    logger.debug() << "Fallback language:" << fallbackCode;

    if (!createTranslator(QLocale(fallbackCode))) {
      logger.warning() << "Loading the fallback locale failed - code:"
                       << fallbackCode;
    }
  }
}

bool Addon::createTranslator(const QLocale& locale) {
  QTranslator* translator = new QTranslator(this);
  m_translators.append(translator);
  QCoreApplication::installTranslator(translator);

  return translator->load(locale, "locale", "_",
                          QFileInfo(m_manifestFileName).dir().filePath("i18n"));
}
