/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "taskscheduler.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
    s_instance->initialize();
  }
  return s_instance;
}

AddonManager::AddonManager(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(AddonManager);
}

AddonManager::~AddonManager() { MVPN_COUNT_DTOR(AddonManager); }

void AddonManager::initialize() {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return;
  }

  if (!validateIndex(readIndex())) {
    logger.debug() << "Unable to validate the index";
  }
}

bool AddonManager::validateIndex(const QByteArray& index) {
  // TODO: signature validation

  QJsonDocument doc = QJsonDocument::fromJson(index);
  if (!doc.isObject()) {
    logger.debug() << "The index must be an object";
    return false;
  }

  QJsonObject obj = doc.object();
  if (obj["api_version"].toString() != "0.1") {
    logger.debug() << "Invalid index file - api_version does not match";
    return false;
  }

  QList<AddonData> addons;
  for (const QJsonValue item : obj["addons"].toArray()) {
    QJsonObject addonObj = item.toObject();

    QString sha256hex = addonObj["sha256"].toString();
    if (sha256hex.isEmpty()) {
      logger.warning() << "Incomplete index - sha256";
      return false;
    }

    if (sha256hex.length() != 64) {
      logger.warning() << "Invalid sha256 hash";
      return false;
    }

    QString addonId = addonObj["id"].toString();
    if (addonId.isEmpty()) {
      logger.warning() << "Incomplete index - addonId";
      return false;
    }

    addons.append(
        {QByteArray::fromHex(sha256hex.toLocal8Bit()), addonId, nullptr});
  }

  for (const AddonData& addonData : addons) {
    if (!m_addons.contains(addonData.m_addonId)) {
      validateAndLoad(addonData.m_addonId, addonData.m_sha256);
      continue;
    }
  }

  return true;
}

bool AddonManager::loadManifest(const QString& manifestFileName,
                                const QByteArray& sha256) {
  Addon* addon = Addon::create(this, manifestFileName);
  if (!addon) {
    logger.warning() << "Unable to create an addon from manifest"
                     << manifestFileName;
    return false;
  }

  m_addons.insert(addon->id(), {sha256, addon->id(), addon});
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

  Addon* addon = m_addons[addonId].m_addon;
  Q_ASSERT(addon);

  m_addons.remove(addonId);
  addon->deleteLater();
}

void AddonManager::retranslate() {
  foreach (const AddonData& addonData, m_addons) {
    // This comment is here to make the linter happy.
    addonData.m_addon->retranslate();
  }
}

// static
bool AddonManager::addonDir(QDir* dir) {
  Q_ASSERT(dir);

  QString addonPath;
#if defined(ADDONS_PATH)
  addonPath = ADDONS_PATH;
#elif defined(MVPN_WINDOWS)
  addonPath = QString("%1/addons").arg(QCoreApplication::applicationDirPath());
#elif defined(MVPN_MACOS)
  addonPath = QString("%1/../Resources/addons")
                  .arg(QCoreApplication::applicationDirPath());
#elif defined(MVPN_IOS)
  addonPath = QString("%1/addons").arg(QCoreApplication::applicationDirPath());
#elif defined(MVPN_ANDROID)
  addonPath = QString("assets:/addons");
#elif defined(MVPN_WASM)
  addonPath = QString(":/addons");
#endif

  if (addonPath.isEmpty()) {
    return false;
  }

  *dir = addonPath;
  return true;
}

// static
QByteArray AddonManager::readIndex() {
  QFile indexFile(":/addons/manifest.json");
  if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to open the addon index";
    return "";
  }

  return indexFile.readAll();
}

// static
void AddonManager::removeAddon(const QString& addonId) {
  QDir dir;
  if (!addonDir(&dir)) {
    return;
  }

  QString addonFileName(QString("%1.rcc").arg(addonId));
  if (!dir.exists(addonFileName)) {
    logger.warning() << "Addon does not exist" << addonFileName;
    return;
  }

  if (!dir.remove(addonFileName)) {
    logger.warning() << "Unable to remove the addon file name";
  }
}

bool AddonManager::validateAndLoad(const QString& addonId,
                                   const QByteArray& sha256, bool checkSha256) {
  logger.debug() << "Load addon" << addonId;

  if (m_addons.contains(addonId)) {
    logger.warning() << "Addon" << addonId << "already loaded";
    return false;
  }

  // Hash validation
  QDir dir;
  if (!addonDir(&dir)) {
    return false;
  }

  QString addonFileName(dir.filePath(QString("%1.rcc").arg(addonId)));
  if (checkSha256) {
    QFile addonFile(addonFileName);
    if (!addonFile.open(QIODevice::ReadOnly)) {
      logger.warning() << "Unable to open the addon file" << addonFileName;
      return false;
    }

    if (QCryptographicHash::hash(addonFile.readAll(),
                                 QCryptographicHash::Sha256) != sha256) {
      logger.warning() << "Addon hash does not match" << addonFileName;
      return false;
    }
  }

  if (!QResource::registerResource(addonFileName,
                                   QString("/addons/%1").arg(addonId))) {
    logger.warning() << "Unable to load resource from file" << addonFileName;
    return false;
  }

  if (!loadManifest(QString(":/addons/%1/manifest.json").arg(addonId),
                    sha256)) {
    QResource::unregisterResource(addonFileName, "/addons");
    return false;
  }

  return true;
}
