/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"
#include "addons/addon.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QResource>
#include <QStandardPaths>

namespace {

Logger logger(LOG_MAIN, "AddonManager");

AddonManager* s_instance = nullptr;

}  // namespace

// static
AddonManager* AddonManager::instance() {
  if (!s_instance) {
    s_instance = new AddonManager(qApp);
    s_instance->loadAll();
  }
  return s_instance;
}

AddonManager::AddonManager(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AddonManager);
}

AddonManager::~AddonManager() { MVPN_COUNT_DTOR(AddonManager); }

void AddonManager::loadAll() {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return;
  }

  QString addonPath;
#if defined(ADDONS_PATH)
  addonPath = ADDONS_PATH;
#elif defined(MVPN_WINDOWS)
  addonPath =
      QString("%1/addons").arg(QCoreApplication::applicationDirPath());  // TODO
#elif defined(MVPN_MACOS)
  addonPath = QString("%1/../Contents/Release/addons")
                  .arg(QCoreApplication::applicationDirPath());
#elif defined(MVPN_IOS)
  addonPath = QString("%1/addons").arg(QCoreApplication::applicationDirPath());
#elif defined(MVPN_ANDROID)
  addonPath = QString("assets:/addons");
#elif defined(MVPN_WASM)
  addonPath = QString(":/addons");
#endif

  logger.debug() << "Loading addon from" << addonPath;

  if (!addonPath.isEmpty()) {
    QDir addonDir(addonPath);
    addonDir.setSorting(QDir::Name);
    loadAll(addonDir);
  }

  if (!Constants::inProduction()) {
    QDir homePath(
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    homePath.cd(".mozillavpn_addons");
    homePath.setSorting(QDir::Name);
    loadAll(homePath);
  }
}

void AddonManager::loadAll(const QDir& path) {
  for (const QString& file :
       path.entryList(QStringList{"*.rcc"}, QDir::Files)) {
    load(path.filePath(file));
  }
}

bool AddonManager::load(const QString& fileName) {
  logger.debug() << "Load addon" << fileName;

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

  if (!loadManifest(QString(":/addons/%1/manifest.json").arg(addonId))) {
    QResource::unregisterResource(fileName, "/addons");
    return false;
  }

  return true;
}

bool AddonManager::loadManifest(const QString& manifestFileName) {
  Addon* addon = Addon::create(this, manifestFileName);
  if (!addon) {
    logger.warning() << "Unable to create an addon from manifest"
                     << manifestFileName;
    return false;
  }

  m_addons.insert(addon->id(), addon);
  return true;
}

void AddonManager::unload(const QString& addonId) {
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

  addon->run();
}

void AddonManager::retranslate() {
  foreach (Addon* addon, m_addons) {
    // This comment is here to make the linter happy.
    addon->retranslate();
  }
}
