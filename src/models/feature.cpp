/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "feature.h"
#include "constants.h"
#include "featureslistcallback.h"
#include "l18nstrings.h"
#include "logger.h"
#include "settingsholder.h"
#include "update/versionapi.h"

#include <QScopeGuard>
#include <QList>

namespace {
Logger logger(LOG_MODEL, "Feature");
QMap<QString, Feature*>* s_featuresHashtable = nullptr;
QList<Feature*>* s_featuresList = nullptr;
}  // namespace

// static
void Feature::maybeInitialize() {
  if (!s_featuresHashtable) {
    s_featuresHashtable = new QMap<QString, Feature*>();

    Q_ASSERT(!s_featuresList);
    s_featuresList = new QList<Feature*>();

#define FEATURE(id, name, isMajor, displayNameId, shortDescId, descId,         \
                imgPath, iconPath, linkUrl, releaseVersion, flippableOn,       \
                flippableOff, otherFeatureDependencies, callback)              \
  new Feature(#id, name, isMajor, displayNameId, shortDescId, descId, imgPath, \
              iconPath, linkUrl, releaseVersion, flippableOn, flippableOff,    \
              otherFeatureDependencies, callback);
#include "featureslist.h"
#undef FEATURE
  }
}

Feature::Feature(const QString& id, const QString& name, bool isMajor,
                 L18nStrings::String displayName_id,
                 L18nStrings::String shortDesc_id, L18nStrings::String desc_id,
                 const QString& imgPath, const QString& iconPath,
                 const QString& linkUrl, const QString& aReleaseVersion,
                 bool flippableOn, bool flippableOff,
                 const QStringList& featureDependencies,
                 std::function<bool()>&& callback)
    : QObject(qApp),
      m_id(id),
      m_name(name),
      m_majorFeature(isMajor),
      m_displayName_id(displayName_id),
      m_shortDescription_id(shortDesc_id),
      m_description_id(desc_id),
      m_imagePath(imgPath),
      m_iconPath(iconPath),
      m_linkUrl(linkUrl),
      m_releaseVersion(aReleaseVersion),
      m_flippableOn(flippableOn),
      m_flippableOff(flippableOff),
      m_featureDependencies(featureDependencies),
      m_callback(callback) {
  logger.debug() << "Initializing feature" << id;

  Q_ASSERT(s_featuresHashtable);
  s_featuresHashtable->insert(m_id, this);
  Q_ASSERT(s_featuresList);
  s_featuresList->append(this);

  auto releaseVersion = VersionApi::stripMinor(aReleaseVersion);
  auto currentVersion = VersionApi::stripMinor(Constants::versionString());

  auto cmp = VersionApi::compareVersions(releaseVersion, currentVersion);
  if (cmp == -1) {
    // Release version < currVersion
    m_released = true;
    m_new = false;
  } else if (cmp == 0) {
    // Release version === currVersion
    m_released = true;
    m_new = true;
  } else {
    // releaseVersion > currVersion
    m_released = false;
    m_new = false;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (m_flippableOn) {
    if (settingsHolder->featuresFlippedOn().contains(m_id)) {
      m_state = FlippedOn;
    }

    connect(settingsHolder, &SettingsHolder::featuresFlippedOnChanged, this,
            &Feature::maybeFlipOnOrOff);
  }

  if (m_flippableOff) {
    if (m_state == DefaultValue &&
        settingsHolder->featuresFlippedOff().contains(m_id)) {
      m_state = FlippedOff;
    }

    connect(settingsHolder, &SettingsHolder::featuresFlippedOffChanged, this,
            &Feature::maybeFlipOnOrOff);
  }
}

Feature::~Feature() {
  s_featuresHashtable->remove(m_id);
  s_featuresList->removeAll(this);
}

// static
const QList<Feature*>& Feature::getAll() {
  maybeInitialize();
  return *s_featuresList;
}

// static
const Feature* Feature::getOrNull(const QString& featureID) {
  maybeInitialize();

  return s_featuresHashtable->value(featureID, nullptr);
}

// static
const Feature* Feature::get(const QString& featureID) {
  maybeInitialize();

  const Feature* feature = getOrNull(featureID);
  if (!feature) {
    logger.error() << "Invalid feature" << featureID;
    Q_ASSERT(false);
    return nullptr;
  };

  return feature;
}

bool Feature::isFlippedOn(bool ignoreCache) const {
  if (!m_flippableOn) {
    return false;
  }

  if (ignoreCache) {
    return SettingsHolder::instance()->featuresFlippedOn().contains(m_id);
  }

  return m_state == FlippedOn;
}

bool Feature::isFlippedOff(bool ignoreCache) const {
  if (!m_flippableOff) {
    return false;
  }

  if (ignoreCache) {
    return SettingsHolder::instance()->featuresFlippedOff().contains(m_id);
  }

  return m_state == FlippedOff;
}

bool Feature::isSupported(bool ignoreCache) const {
  if (isFlippedOn(ignoreCache)) {
    logger.debug() << "Flipped On" << m_id;
    return true;
  }

  if (isFlippedOff(ignoreCache)) {
    logger.debug() << "Flipped Off" << m_id;
    return false;
  }

  return isSupportedIgnoringFlip();
}

bool Feature::isSupportedIgnoringFlip() const {
  if (!m_released) {
    return false;
  }

  if (!m_callback()) {
    return false;
  }

  for (const QString& featureID : m_featureDependencies) {
    const Feature* feature = Feature::get(featureID);
    if (!feature->isSupported()) {
      return false;
    }
  }

  return true;
}

QString Feature::displayName() const {
  return L18nStrings::instance()->t(m_displayName_id);
}
QString Feature::description() const {
  return L18nStrings::instance()->t(m_description_id);
}
QString Feature::shortDescription() const {
  return L18nStrings::instance()->t(m_shortDescription_id);
}

void Feature::maybeFlipOnOrOff() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  State newState = DefaultValue;
  if (settingsHolder->featuresFlippedOn().contains(m_id)) {
    newState = FlippedOn;
  } else if (settingsHolder->featuresFlippedOff().contains(m_id)) {
    newState = FlippedOff;
  }

  if (newState == m_state) {
    // Something else has changed. Let's see if we have to disable this feature
    // because one of the dependencies has changed.
    if (newState == FlippedOn) {
      for (const QString& featureID : m_featureDependencies) {
        Feature* feature = s_featuresHashtable->value(featureID, nullptr);
        Q_ASSERT(feature);

        // Let's check the support ignoring the cache (m_state)
        // because maybe this feature doesn't know yet that it has been
        // disabled.
        if (feature->isSupported(true)) continue;

        QStringList featuresFlippedOn =
            SettingsHolder::instance()->featuresFlippedOn();
        featuresFlippedOn.removeAll(m_id);
        SettingsHolder::instance()->setFeaturesFlippedOn(featuresFlippedOn);
        break;
      }
    }
    return;
  }

  if (newState != FlippedOn) {
    m_state = newState;
    emit supportedChanged();
    return;
  }

  // Let's set it before checking other features to break cycles.
  m_state = newState;

  QList<Feature*> featuresToFlipOnAndCheck;
  for (const QString& featureID : m_featureDependencies) {
    Feature* feature = s_featuresHashtable->value(featureID, nullptr);
    Q_ASSERT(feature);

    if (feature->isSupported(true)) continue;

    if (!feature->m_flippableOn) {
      logger.debug() << "Unable to activate feature" << id()
                     << "because feature" << feature->id()
                     << "cannot be enabled in dev mode";
      m_state = DefaultValue;
      return;
    }

    featuresToFlipOnAndCheck.append(feature);
  }

  if (!featuresToFlipOnAndCheck.isEmpty()) {
    QStringList featuresFlippedOn =
        SettingsHolder::instance()->featuresFlippedOn();

    for (Feature* feature : featuresToFlipOnAndCheck) {
      if (!featuresFlippedOn.contains(feature->m_id)) {
        featuresFlippedOn.append(feature->m_id);
      }
    }

    SettingsHolder::instance()->setFeaturesFlippedOn(featuresFlippedOn);

    for (Feature* feature : featuresToFlipOnAndCheck) {
      if (!feature->isSupported()) {
        logger.debug() << "Unable to activate feature" << id()
                       << "because feature" << feature->id()
                       << "cannot be enabled";
        m_state = DefaultValue;
        return;
      }
    }
  }

  emit supportedChanged();
}
