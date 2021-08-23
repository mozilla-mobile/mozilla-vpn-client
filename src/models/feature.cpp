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

Feature::Feature(QString id, L18nStrings::String name_id, bool isMajor,
                 L18nStrings::String shortDesc_id, L18nStrings::String desc_id,
                 QString imgPath, QString iconPath, QString aReleaseVersion,
                 bool devModeWriteable)
    : QObject(qApp),
      m_id(id),
      m_name_id(name_id),
      m_majorFeature(isMajor),
      m_shortDescription_id(shortDesc_id),
      m_description_id(desc_id),
      m_imagePath(imgPath),
      m_iconPath(iconPath),
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
const Feature* Feature::get(const QString& featureID) {
  if (!s_features->contains(featureID)) {
    logger.error() << "Invalid feature" << featureID;
    Q_ASSERT(false);
    return nullptr;
  };

  return s_features->value(featureID, nullptr);
}

bool Feature::isDevModeEnabled() const {
  if (!m_devModeWriteable) {
    return false;
  }
  const auto settings = SettingsHolder::instance();
  return settings->hasDevModeFeatureFlag(m_id);
}

bool Feature::isSupported() const {
  logger.debug() << "Check feature " << m_id;
  if (isDevModeEnabled()) {
    logger.debug() << "Devmode Enabled " << m_id;
    return true;
  }
  if (!m_released) {
    logger.debug() << "Not released " << m_id;
    return false;
  }
  logger.debug() << "Check Support" << m_id;
  return checkSupportCallback();
}

QString Feature::name() const { return L18nStrings::instance()->tr(m_name_id); }
QString Feature::description() const {
  return L18nStrings::instance()->tr(m_description_id);
}
QString Feature::shortDescription() const {
  return L18nStrings::instance()->tr(m_shortDescription_id);
}

QVariant Feature::data(int role) const {
  switch ((ModelRoles)role) {
    case RoleId:
      return m_id;
    case RoleName:
      return name();
    case RoleDescription:
      return description();
    case RoleShortDescription:
      return shortDescription();
    case RoleImagePath:
      return m_imagePath;
    case RoleIconPath:
      return m_iconPath;
    case RoleReleased:
      return m_released;
    case RoleSupported:
      return isSupported();
    case RoleDevModeWriteable:
      return m_devModeWriteable;
    case RoleDevModeEnabled:
      return isDevModeEnabled();
    case RoleNew:
      return m_new;
    case RoleMajor:
      return m_majorFeature;
    default:
      return QVariant();
  }
};

QHash<int, QByteArray> Feature::roleNames() {
  // TODO: we probably could cleanup & remove the role enum
  // and just get this via the Q_metaobject
  return QHash<int, QByteArray>({{RoleId, "id"},
                                 {RoleName, "name"},
                                 {RoleDescription, "description"},
                                 {RoleShortDescription, "shortDescription"},
                                 {RoleImagePath, "imagePath"},
                                 {RoleIconPath, "iconPath"},
                                 {RoleReleased, "featureReleased"},
                                 {RoleNew, "isNew"},
                                 {RoleMajor, "isMajor"},
                                 {RoleSupported, "supported"},
                                 {RoleDevModeWriteable, "devModeWriteable"},
                                 {RoleDevModeEnabled, "devModeEnabled"}});
}
