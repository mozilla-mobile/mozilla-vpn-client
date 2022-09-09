/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addondirectory.h"
#include "addonindex.h"
#include "addonmanager.h"
#include "addons/addonmessage.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "models/feature.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "tasks/addon/taskaddon.h"
#include "tasks/function/taskfunction.h"
#include "taskscheduler.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QResource>
#include <QQmlEngine>
#include <QSaveFile>

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

// static
QString AddonManager::addonServerAddress() {
  if (Constants::inProduction()) {
    return Constants::ADDON_PRODUCTION_URL;
  }

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->addonCustomServer()) {
    return settingsHolder->addonCustomServerAddress();
  }

  return Constants::ADDON_STAGING_URL;
}

AddonManager::AddonManager(QObject* parent)
    : QAbstractListModel(parent), m_addonIndex(&m_addonDirectory) {
  MVPN_COUNT_CTOR(AddonManager);
}

AddonManager::~AddonManager() { MVPN_COUNT_DTOR(AddonManager); }

void AddonManager::initialize() {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return;
  }

  // Load on disk addons, doing this will initialize the addons directory
  QList<AddonData> addons;
  if (m_addonIndex.getOnDiskAddonsList(&addons)) {
    updateAddonsList(addons);
  }

  // Listen for updates in the addons list
  connect(&m_addonIndex, &AddonIndex::indexUpdated, this,
          &AddonManager::updateAddonsList);
}

void AddonManager::updateIndex(const QByteArray& index,
                               const QByteArray& indexSignature) {
  m_addonIndex.update(index, indexSignature);
}

void AddonManager::updateAddonsList(QList<AddonData> addons) {
  logger.debug() << "Updating addons list";

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

  bool taskAdded = false;

  // Fetch new addons
  for (const AddonData& addonData : addons) {
    if (!m_addons.contains(addonData.m_addonId) &&
        validateAndLoad(addonData.m_addonId, addonData.m_sha256)) {
      continue;
    }

    Q_ASSERT(m_addons.contains(addonData.m_addonId));

    if (m_addons[addonData.m_addonId].m_sha256 != addonData.m_sha256) {
      TaskScheduler::scheduleTask(
          new TaskAddon(addonData.m_addonId, addonData.m_sha256));
      taskAdded = true;
    }
  }

  if (!m_loadCompleted) {
    if (taskAdded) {
      TaskScheduler::scheduleTask(new TaskFunction(
          [this]() {
            m_loadCompleted = true;
            emit loadCompletedChanged();
          },
          false));
    } else {
      m_loadCompleted = true;
      emit loadCompletedChanged();
    }
  }
}

bool AddonManager::loadManifest(const QString& manifestFileName) {
  Addon* addon = Addon::create(this, manifestFileName);
  if (!addon) {
    logger.warning() << "Unable to create an addon from manifest"
                     << manifestFileName;
    return false;
  }

  bool addonEnabled = addon->enabled();
  if (addonEnabled) {
    beginResetModel();
  }

  Q_ASSERT(m_addons.contains(addon->id()));
  m_addons[addon->id()].m_addon = addon;

  if (addonEnabled) {
    endResetModel();
  }

  connect(addon, &Addon::conditionChanged, this, [this, addon](bool enabled) {
    int pos = 0;
    for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
         i != m_addons.constEnd(); ++i) {
      if (!i.value().m_addon) continue;
      if (i.value().m_addon != addon) {
        if (i.value().m_addon->enabled()) ++pos;
        continue;
      }
      if (!enabled) {
        beginRemoveRows(QModelIndex(), pos, pos);
        removeRow(pos);
        endRemoveRows();
      } else {
        beginInsertRows(QModelIndex(), pos, pos);
        insertRow(pos);
        endInsertRows();
      }
      break;
    }
  });

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

  if (!addon) {
    m_addons.remove(addonId);
    return;
  }

  bool addonEnabled = addon->enabled();
  if (addonEnabled) {
    beginResetModel();
  }

  m_addons.remove(addonId);

  if (addonEnabled) {
    endResetModel();
  }

  QDir dir;
  if (m_addonDirectory.getDirectory(&dir)) {
    QString addonFileName(QString("%1.rcc").arg(addonId));
    QString addonFilePath(dir.filePath(addonFileName));
    QResource::unregisterResource(addonFilePath, mountPath(addonId));
  }

  addon->deleteLater();
}

void AddonManager::retranslate() {
  foreach (const AddonData& addonData, m_addons) {
    if (addonData.m_addon) {
      // This comment is here to make the linter happy.
      addonData.m_addon->retranslate();
    }
  }
}

// static
void AddonManager::removeAddon(const QString& addonId) {
  QString addonFileName(QString("%1.rcc").arg(addonId));
  instance()->m_addonDirectory.deleteFile(addonFileName);
}

bool AddonManager::validateAndLoad(const QString& addonId,
                                   const QByteArray& sha256, bool checkSha256) {
  logger.debug() << "Load addon" << addonId;

#ifdef MVPN_WASM
  if (addonId.startsWith("message_")) {
    logger.debug() << "Skipping the message addon";
    return true;
  }
#endif

  if (m_addons.contains(addonId)) {
    logger.warning() << "Addon" << addonId << "already loaded";
    return false;
  }

  m_addons.insert(addonId, {QByteArray(), addonId, nullptr});

  QString addonFileName(QString("%1.rcc").arg(addonId));

  QDir dir;
  if (!m_addonDirectory.getDirectory(&dir)) {
    return false;
  }
  QString addonFilePath(dir.filePath(addonFileName));

  // Hash validation
  if (checkSha256) {
    QByteArray addonFileContents;

    if (!m_addonDirectory.readFile(addonFileName, &addonFileContents)) {
      return false;
    }

    if (QCryptographicHash::hash(addonFileContents,
                                 QCryptographicHash::Sha256) != sha256) {
      logger.warning() << "Addon hash does not match" << addonFilePath;
      return false;
    }
  }

  m_addons[addonId].m_sha256 = sha256;
  QString addonMountPath = mountPath(addonId);

  if (!QResource::registerResource(addonFilePath, addonMountPath)) {
    logger.warning() << "Unable to load resource from file" << addonFilePath;
    return false;
  }

  if (!loadManifest(QString(":%1/manifest.json").arg(addonMountPath))) {
    QResource::unregisterResource(addonFilePath, addonMountPath);
    return false;
  }

  return true;
}

void AddonManager::storeAndLoadAddon(const QByteArray& addonData,
                                     const QString& addonId,
                                     const QByteArray& sha256) {
  logger.debug() << "Store and load addon" << addonId;

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

  QString addonFileName(QString("%1.rcc").arg(addonId));
  if (!m_addonDirectory.writeToFile(addonFileName, addonData)) {
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
  int count = 0;
  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    if (i.value().m_addon && i.value().m_addon->enabled()) {
      ++count;
    }
  }
  return count;
}

QVariant AddonManager::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case AddonRole: {
      int row = index.row();
      for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
           i != m_addons.constEnd(); ++i) {
        if (!i.value().m_addon || !i.value().m_addon->enabled()) {
          continue;
        }
        if (row == 0) {
          return QVariant::fromValue(i.value().m_addon);
        }
        --row;
      }
    }

    default:
      return QVariant();
  }
}

void AddonManager::forEach(std::function<void(Addon*)>&& a_callback) {
  std::function<void(Addon*)> callback = std::move(a_callback);
  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    if (i.value().m_addon && i.value().m_addon->enabled()) {
      callback(i.value().m_addon);
    }
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
    if (!i.value().m_addon || !i.value().m_addon->enabled()) {
      continue;
    }

    QJSValueList arguments;
    arguments.append(engine->toScriptValue(i.value().m_addon));
    QJSValue retValue = filterCallback.call(arguments);
    if (retValue.toBool()) {
      return i.value().m_addon;
    }
  }

  return nullptr;
}

QJSValue AddonManager::reduce(QJSValue callback, QJSValue initialValue) const {
  if (!callback.isCallable()) {
    logger.error() << "AddonManager.reduce must receive a callable JS value";
    return initialValue;
  }

  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  Q_ASSERT(engine);

  QJSValue reducedValue = initialValue;

  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    if (!i.value().m_addon || !i.value().m_addon->enabled()) {
      continue;
    }

    QJSValueList arguments;
    arguments.append(engine->toScriptValue(i.value().m_addon));
    arguments.append(reducedValue);
    reducedValue = callback.call(arguments);
  }

  return reducedValue;
}

// Undismisses any dismissed messages and marks all messages as unread
void AddonManager::reinstateMessages() const {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  settingsHolder->clearAddonSettings(ADDON_MESSAGE_SETTINGS_GROUP);
}

#ifdef UNIT_TEST
QStringList AddonManager::addonIds() const { return m_addons.keys(); }
#endif

// static
QString AddonManager::mountPath(const QString& addonId) {
  return QString("/addons/%1").arg(addonId);
}
