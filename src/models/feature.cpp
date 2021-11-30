/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "feature.h"
#include "l18nstrings.h"
#include "logger.h"
#include "settingsholder.h"
#include "update/versionapi.h"

#include <QScopeGuard>
#include <QList>

namespace {
Logger logger(LOG_MODEL, "Feature");
QMap<QString, Feature*>* s_features = nullptr;
}  // namespace

Feature::Feature(const QString& id, const QString& name, bool isMajor,
                 L18nStrings::String displayName_id,
                 L18nStrings::String shortDesc_id, L18nStrings::String desc_id,
                 const QString& imgPath, const QString& iconPath,
                 const QString& linkUrl, const QString& aReleaseVersion,
                 bool devModeWriteable)
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
      m_devModeWriteable(devModeWriteable) {
  logger.debug() << "Initializing feature" << id;

  if (!s_features) {
    s_features = new QMap<QString, Feature*>();
  }

  s_features->insert(m_id, this);

  auto releaseVersion = VersionApi::stripMinor(aReleaseVersion);
  auto currentVersion = VersionApi::stripMinor(APP_VERSION);

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

  if (m_devModeWriteable) {
    connect(SettingsHolder::instance(),
            &SettingsHolder::devModeFeatureFlagsChanged, this,
            [this]() { emit supportedChanged(); });
  }
}

// static
QList<Feature*> Feature::getAll() { return s_features->values(); }

// static
const Feature* Feature::getOrNull(const QString& featureID) {
  return s_features->value(featureID, nullptr);
}

// static
const Feature* Feature::get(const QString& featureID) {
  const Feature* feature = getOrNull(featureID);
  if (!feature) {
    logger.error() << "Invalid feature" << featureID;
    Q_ASSERT(false);
    return nullptr;
  };

  return feature;
}

bool Feature::isDevModeEnabled() const {
  if (!m_devModeWriteable) {
    return false;
  }

  return SettingsHolder::instance()->devModeFeatureFlags().contains(m_id);
}

bool Feature::isSupported() const {
  if (isDevModeEnabled()) {
    logger.debug() << "Devmode Enabled " << m_id;
    return true;
  }
  if (!m_released) {
    return false;
  }
  return checkSupportCallback();
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
