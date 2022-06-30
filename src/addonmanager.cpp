/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "qmlengineholder.h"
#include "signature.h"
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
constexpr const char* ADDON_INDEX_SIGNATURE_FILENAME = "manifest.json.sign";

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

AddonManager::AddonManager(QObject* parent) : QAbstractListModel(parent) {
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

  QByteArray index;
  QByteArray indexSignature;
  if (!readIndex(index, indexSignature)) {
    logger.info() << "Unable to read the addon index";
    return;
  }

  if (!validateIndex(index, indexSignature)) {
    logger.debug() << "Unable to validate the index";
  }
}

void AddonManager::updateIndex(const QByteArray& index,
                               const QByteArray& indexSignature) {
  QByteArray currentIndex;
  QByteArray currentIndexSignature;
  if (readIndex(currentIndex, currentIndexSignature) && currentIndex == index &&
      currentIndexSignature == indexSignature) {
    logger.debug() << "The index has not changed";
    return;
  }

  if (!validateIndex(index, indexSignature)) {
    logger.debug() << "Unable to validate the index";
    return;
  }

  writeIndex(index, indexSignature);
}

bool AddonManager::validateIndex(const QByteArray& index,
                                 const QByteArray& indexSignature) {
  QFile publicKeyFile(Constants::addonPublicKeyFile());
  if (!publicKeyFile.open(QIODevice::ReadOnly)) {
    logger.warning() << "Unable to open the addon public key file";
    return false;
  }

  QByteArray publicKey = publicKeyFile.readAll();
  if (!Signature::verify(publicKey, index, indexSignature)) {
    logger.warning() << "Unable to verify the signature of the addon index";
    return false;
  }

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
  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
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

  beginResetModel();
  m_addons.insert(addon->id(), {sha256, addon->id(), addon});
  endResetModel();
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

  beginResetModel();
  m_addons.remove(addonId);
  endResetModel();

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
bool AddonManager::readIndex(QByteArray& index, QByteArray& indexSignature) {
  QDir dir;
  if (!addonDir(&dir)) {
    return false;
  }

  // Index file
  {
    QFile indexFile(dir.filePath(ADDON_INDEX_FILENAME));
    if (!indexFile.open(QIODevice::ReadOnly)) {
      logger.warning() << "Unable to open the addon index";
      return false;
    }

    index = indexFile.readAll();
  }

  // Index signature file
  {
    QFile indexSignatureFile(dir.filePath(ADDON_INDEX_SIGNATURE_FILENAME));
    if (!indexSignatureFile.open(QIODevice::ReadOnly)) {
      logger.warning() << "Unable to open the addon index signature";
      return false;
    }

    indexSignature = indexSignatureFile.readAll();
  }

  return true;
}

// static
void AddonManager::writeIndex(const QByteArray& index,
                              const QByteArray& indexSignature) {
  QDir dir;
  if (!addonDir(&dir)) {
    return;
  }

  // Index file
  {
    QFile indexFile(dir.filePath(ADDON_INDEX_FILENAME));
    if (!indexFile.open(QIODevice::WriteOnly)) {
      logger.warning() << "Unable to open the addon index file";
      return;
    }

    if (!indexFile.write(index)) {
      logger.warning() << "Unable to write the addon index file";
    }
  }

  // Index signature file
  {
    QFile indexSignatureFile(dir.filePath(ADDON_INDEX_SIGNATURE_FILENAME));
    if (!indexSignatureFile.open(QIODevice::WriteOnly)) {
      logger.warning() << "Unable to open the addon index signature file";
      return;
    }

    if (!indexSignatureFile.write(indexSignature)) {
      logger.warning() << "Unable to write the addon index signature file";
    }
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

QHash<int, QByteArray> AddonManager::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[AddonRole] = "addon";
  return roles;
}

int AddonManager::rowCount(const QModelIndex&) const {
  return m_addons.count();
}

QVariant AddonManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case AddonRole:
      return QVariant::fromValue(
          m_addons[m_addons.keys().at(index.row())].m_addon);

    default:
      return QVariant();
  }
}

void AddonManager::forEach(std::function<void(Addon*)>&& a_callback) {
  std::function<void(Addon*)> callback = std::move(a_callback);
  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    callback(i.value().m_addon);
  }
}

Addon* AddonManager::pick(QJSValue filterCallback) const {
  if (!filterCallback.isCallable()) {
    logger.error() << "AddonManager.pick must receive a callable JS value";
    return nullptr;
  }

  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  Q_ASSERT(engine);

  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    QJSValueList arguments;
    arguments.append(engine->toScriptValue(i.value().m_addon));
    QJSValue retValue = filterCallback.call(arguments);
    if (retValue.toBool()) {
      return i.value().m_addon;
    }
  }

  return nullptr;
}
