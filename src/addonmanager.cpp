/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "models/feature.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QResource>
#include <QScopeGuard>

namespace {

Logger logger(LOG_MAIN, "AddonManager");

AddonManager* s_instance = nullptr;

}  // namespace

// static
AddonManager* AddonManager::instance() {
  if (!s_instance) {
    s_instance = new AddonManager(qApp);
  }
  return s_instance;
}

AddonManager::AddonManager(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AddonManager);
}

AddonManager::~AddonManager() { MVPN_COUNT_DTOR(AddonManager); }

bool AddonManager::load(const QString& fileName) {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return false;
  }

  QString addonId = QFileInfo(fileName).baseName();
  if (m_addons.contains(addonId)) {
    logger.warning() << "Addon" << addonId << "already loaded";
    return false;
  }

  if (!QResource::registerResource(fileName, "/addons")) {
    logger.warning() << "Unable to load resource from file" << fileName;
    return false;
  }

  auto guard =
      qScopeGuard([&] { QResource::unregisterResource(fileName, "/addons"); });

  QFile file(QString(":/addons/%1/manifest.json").arg(addonId));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to read the addon manifest of" << addonId;
    return false;
  }

  QJsonDocument json = QJsonDocument::fromJson(file.readAll());
  if (!json.isObject()) {
    logger.warning() << "The manifest must be a JSON document" << addonId;
    return false;
  }

  QJsonObject obj = json.object();

  QString version = obj["version"].toString();
  if (version.isEmpty()) {
    logger.warning() << "No version in the manifest" << addonId;
    return false;
  }

  if (version != "0.1") {
    logger.warning() << "Unsupported version" << version << addonId;
    return false;
  }

  QString name = obj["name"].toString();
  if (name.isEmpty()) {
    logger.warning() << "No name in the manifest" << addonId;
    return false;
  }

  QString type = obj["type"].toString();
  if (type.isEmpty()) {
    logger.warning() << "No type in the manifest" << addonId;
    return false;
  }

  Addon::AddonType addonType;
  QString qmlFileName;

  if (type == "demo") {
    addonType = Addon::AddonTypeDemo;
    QString qml = obj["qml"].toString();
    if (qml.isEmpty()) {
      logger.warning() << "No qml in the manifest" << addonId;
      return false;
    }

    qmlFileName = QString(":/addons/%1/%2").arg(addonId).arg(qml);
    if (!QFile::exists(qmlFileName)) {
      logger.warning() << "Unable to load the qml entry" << qmlFileName << qml
                       << addonId;
      return false;
    }
  } else if (type == "i18n") {
    addonType = Addon::AddonTypeI18n;
  } else {
    logger.warning() << "Unsupported type" << type << addonId;
    return false;
  }

  guard.dismiss();

  Addon* addon =
      new Addon(this, addonType, fileName, addonId, name, qmlFileName);
  m_addons.insert(addonId, addon);
  return true;
}

void AddonManager::unload(const QString& addonId) {
  if (!m_addons.contains(addonId)) {
    logger.warning() << "No addon with id" << addonId;
    return;
  }

  Addon* addon = m_addons[addonId];
  Q_ASSERT(addon);

  QResource::unregisterResource(addon->fileName(), "/addons");

  emit unloadAddon(addonId);
  m_addons.remove(addonId);

  addon->deleteLater();
}

void AddonManager::run(const QString& addonId) {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return;
  }

  if (!m_addons.contains(addonId)) {
    logger.warning() << "No addon with id" << addonId;
    return;
  }

  Addon* addon = m_addons[addonId];
  Q_ASSERT(addon);

  switch (addon->type()) {
    case Addon::AddonTypeDemo:
      emit runAddon(addon);
      break;

    case Addon::AddonTypeI18n:
      emit Localizer::instance()->codeChanged();
      break;
  }
}

void AddonManager::retranslate() {
  foreach (Addon* addon, m_addons) {
    // This comment is here to make the linter happy.
    addon->retranslate();
  }
}
