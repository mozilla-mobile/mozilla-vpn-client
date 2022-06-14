/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "taskscheduler.h"
#include "tasks/addon/taskaddon.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QResource>
#include <QStandardPaths>

constexpr const char* ADDON_FOLDER = "addons";
constexpr const char* ADDON_INDEX_FILENAME = "manifest.json";

namespace {

Logger logger(LOG_MAIN, "AddonManager");

AddonManager* s_instance = nullptr;

QString rootAppFolder() {
#ifdef MVPN_WASM
  // https://wiki.qt.io/Qt_for_WebAssembly#Files_and_local_file_system_access
  return "/";
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}

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

  // Initialization of the addon folder.
  {
    QDir addonDir(rootAppFolder());
    if (!addonDir.exists(ADDON_FOLDER) && !addonDir.mkdir(ADDON_FOLDER)) {
      logger.info() << "Unable to create the addon folder";
      return;
    }
  }

  if (!validateIndex(readIndex())) {
    logger.debug() << "Unable to validate the index";
  }
}

void AddonManager::updateIndex(const QByteArray& index) {
  QByteArray currentIndex = readIndex();

  if (currentIndex == index) {
    logger.debug() << "The index has not changed";
    return;
  }

  if (!validateIndex(index)) {
    logger.debug() << "Unable to validate the index";
    return;
  }

  writeIndex(index);
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

  // Remove unknown addons
  QStringList addonsToBeRemoved;
  for (QHash<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    bool found = false;
    for (const AddonData& addonData : addons) {
      if (addonData.m_addonId == i.key()) {
        found = true;
        break;
      }
    }

    if (found) continue;
    addonsToBeRemoved.append(i.key());
  }

  for (const QString& addonId : addonsToBeRemoved) {
    unload(addonId);
    removeAddon(addonId);
  }

  // Fetch new addons
  for (const AddonData& addonData : addons) {
    if (!m_addons.contains(addonData.m_addonId) &&
        validateAndLoad(addonData.m_addonId, addonData.m_sha256)) {
      Q_ASSERT(m_addons.contains(addonData.m_addonId));
      Q_ASSERT(m_addons[addonData.m_addonId].m_sha256 == addonData.m_sha256);
      continue;
    }

    if (!m_addons.contains(addonData.m_addonId) ||
        m_addons[addonData.m_addonId].m_sha256 != addonData.m_sha256) {
      TaskScheduler::scheduleTask(
          new TaskAddon(addonData.m_addonId, addonData.m_sha256));
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

  QDir addonDir(rootAppFolder());
  if (!addonDir.exists(ADDON_FOLDER)) {
    return false;
  }

  if (!addonDir.cd(ADDON_FOLDER)) {
    logger.warning() << "Unable to open the addons folder";
    return false;
  }

  *dir = addonDir;
  return true;
}

// static
QByteArray AddonManager::readIndex() {
  QDir dir;
  if (!addonDir(&dir)) {
    return "";
  }

  QFile indexFile(dir.filePath(ADDON_INDEX_FILENAME));
  if (!indexFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logger.warning() << "Unable to open the addon index";
    return "";
  }

  return indexFile.readAll();
}

// static
void AddonManager::writeIndex(const QByteArray& index) {
  QDir dir;
  if (!addonDir(&dir)) {
    return;
  }

  QFile indexFile(dir.filePath(ADDON_INDEX_FILENAME));
  if (!indexFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    logger.warning() << "Unable to open the addon index";
    return;
  }

  if (!indexFile.write(index)) {
    logger.warning() << "Unable to write the addon file";
  }
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

void AddonManager::storeAndLoadAddon(const QByteArray& addonData,
                                     const QString& addonId,
                                     const QByteArray& sha256) {
  // Maybe we have to replace an existing addon. Let's start removing it.
  if (m_addons.contains(addonId)) {
    unload(addonId);
    removeAddon(addonId);
  }

  if (QCryptographicHash::hash(addonData, QCryptographicHash::Sha256) !=
      sha256) {
    logger.warning() << "Invalid addon hash";
    return;
  }

  QDir dir;
  if (!addonDir(&dir)) {
    return;
  }

  QString addonFileName(dir.filePath(QString("%1.rcc").arg(addonId)));
  QFile addonFile(addonFileName);
  if (!addonFile.open(QIODevice::WriteOnly)) {
    logger.warning() << "Unable to open the addon file" << addonFileName;
    return;
  }

  if (!addonFile.write(addonData)) {
    logger.warning() << "Unable to write the addon file";
    return;
  }

  if (!validateAndLoad(addonId, sha256, false)) {
    logger.warning() << "Unable to load the addon";
  }
}
