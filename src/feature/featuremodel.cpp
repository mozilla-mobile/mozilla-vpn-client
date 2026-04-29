/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "featuremodel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QQmlEngine>
#include <array>

#include "logger.h"
#include "settingsholder.h"

using namespace Feature;

namespace {
FeatureModel* s_instance = nullptr;
Logger logger("FeatureModel");
}  // namespace

FeatureModel::FeatureModel() { initialize(); }

FeatureModel* FeatureModel::instance() {
  if (!s_instance) {
    s_instance = new FeatureModel();
  }
  return s_instance;
}

#ifdef UNIT_TEST
void FeatureModel::testCleanup() {
  delete s_instance;
  s_instance = nullptr;
}
#endif

void FeatureModel::initialize() {
  if (m_initialized) return;
  m_initialized = true;

  for (size_t i = 0; i < std::size(s_exposedFeatures); ++i) {
    m_proxies[i].init(&s_exposedFeatures[i]);
  }

  SettingsHolder* sh = SettingsHolder::instance();
  auto notifyFlipChange = [this]() {
    for (auto& proxy : m_proxies) {
      if (proxy.isOverridable()) {
        emit proxy.supportedChanged();
      }
    }
    emit dataChanged(createIndex(0, 0),
                     createIndex(std::size(s_exposedFeatures) - 1, 0));
  };
  connect(sh, &SettingsHolder::featuresFlippedOnChanged, this,
          notifyFlipChange);
  connect(sh, &SettingsHolder::featuresFlippedOffChanged, this,
          notifyFlipChange);
}

FeatureProxy* FeatureModel::proxyForId(const QString& id) {
  for (auto& proxy : m_proxies) {
    if (proxy.id() == id) return &proxy;
  }
  return nullptr;
}

QHash<int, QByteArray> FeatureModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[FeatureRole] = "feature";
  return roles;
}

int FeatureModel::rowCount(const QModelIndex&) const {
  return std::size(s_exposedFeatures);
}

QVariant FeatureModel::data(const QModelIndex& index, int role) const {
  if (role != FeatureRole || index.row() < 0 ||
      index.row() >= static_cast<int>(std::size(s_exposedFeatures))) {
    return QVariant();
  }
  auto* proxy = const_cast<FeatureProxy*>(&m_proxies[index.row()]);
  return QVariant::fromValue(static_cast<QObject*>(proxy));
}

QObject* FeatureModel::get(const QString& feature) {
  auto* proxy = proxyForId(feature);
  if (!proxy) {
    logger.error() << "Feature not found:" << feature;
    return nullptr;
  }
  QQmlEngine::setObjectOwnership(proxy, QQmlEngine::CppOwnership);
  return proxy;
}

bool FeatureModel::isEnabledById(const QString& id) const {
  for (const auto& entry : s_exposedFeatures) {
    if (id == Feature::getId(entry)) return Feature::isEnabled(entry);
  }
  logger.error() << "Unknown feature ID:" << id;
  return false;
}

void FeatureModel::toggle(const QString& featureId) {
  logger.debug() << "Toggle feature" << featureId;

  auto* proxy = proxyForId(featureId);
  if (!proxy) {
    logger.error() << "Feature" << featureId << "not found";
    return;
  }
  if (!proxy->isOverridable()) {
    logger.error() << "Feature" << featureId << "is not overridable";
    return;
  }

  const auto* f = std::get<const OverridableFeature*>(*proxy->feature());
  Feature::toggle(*f, !isEnabled(*f));
}

// static
QHash<QString, bool> FeatureModel::parseFeatures(const QByteArray& data,
                                                 bool acceptExperiments) {
  QHash<QString, bool> overrides;
  QJsonObject json = QJsonDocument::fromJson(data).object();

  if (json.contains("featuresOverwrite")) {
    QJsonObject obj = json["featuresOverwrite"].toObject();
    for (const QString& key : obj.keys()) {
      QJsonValue value = obj.value(key);
      if (!value.isBool()) {
        logger.error() << "Invalid feature value for:" << key;
        continue;
      }
      overrides.insert(key, value.toBool());
    }
  }

  if (acceptExperiments && json.contains("experimentalFeatures")) {
    QJsonObject obj = json["experimentalFeatures"].toObject();
    for (const QString& key : obj.keys()) {
      if (!obj.value(key).isObject()) {
        logger.error() << "Invalid experimental feature format:" << key;
        continue;
      }
      overrides.insert(key, true);
    }
  }
  return overrides;
}

void FeatureModel::updateFeatureList(const QByteArray& data) {
  auto overrides = parseFeatures(data, true);

  for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it) {
    auto* proxy = proxyForId(it.key());
    if (!proxy) {
      logger.warning() << "Unknown feature in server response:" << it.key();
      continue;
    }
    if (!proxy->isOverridable()) {
      logger.warning() << "Feature" << it.key() << "is not overridable";
      continue;
    }

    const auto* f = std::get<const OverridableFeature*>(*proxy->feature());
    Feature::toggle(*f, it.value());
  }
}
