/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonmanager.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcessEnvironment>
#include <QQmlEngine>
#include <QResource>
#include <QSaveFile>

#include "addondirectory.h"
#include "addonindex.h"
#include "addons/addonmessage.h"
#include "context/constants.h"
#include "context/qmlengineholder.h"
#include "feature/feature.h"
#include "leakdetector.h"
#include "logging/logger.h"
#include "settingsholder.h"
#include "tasks/addon/taskaddon.h"
#include "tasks/addonindex/taskaddonindex.h"
#include "tasks/function/taskfunction.h"
#include "taskscheduler.h"

constexpr const char* MVPN_ENV_SKIP_ADDON_SIGNATURE =
    "MVPN_SKIP_ADDON_SIGNATURE";

namespace {
Logger logger("AddonManager");
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
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  if (settingsHolder->addonCustomServer()) {
    return settingsHolder->addonCustomServerAddress();
  }

  return Constants::addonBaseUrl();
}

AddonManager::AddonManager(QObject* parent)
    : QAbstractListModel(parent), m_addonIndex(&m_addonDirectory) {
  MZ_COUNT_CTOR(AddonManager);
}

AddonManager::~AddonManager() { MZ_COUNT_DTOR(AddonManager); }

void AddonManager::initialize() {
  if (!Feature::get(Feature::Feature_addon)->isSupported()) {
    logger.warning() << "Addons disabled by feature flag";
    return;
  }

  // Load on disk addons, doing this will initialize the addons directory
  QList<AddonData> addons;
  if (m_addonIndex.getOnDiskAddonsList(&addons)) {
    updateAddonsList(true, addons);
  }

  // Listen for updates in the addons list
  connect(&m_addonIndex, &AddonIndex::indexUpdated, this,
          &AddonManager::updateAddonsList);
}

void AddonManager::updateIndex(bool status, const QByteArray& index,
                               const QByteArray& indexSignature) {
  if (!status) {
    logger.debug() << "Failed to update index.";
    loadCompleted();
    return;
  }

  m_addonIndex.update(index, indexSignature);
}

void AddonManager::updateAddonsList(bool status, QList<AddonData> addons) {
  logger.debug() << "Updating addons list. Status:" << status
                 << "Addons:" << addons.count();

  if (!status) {
    logger.debug() << "Failed to update the addons list";
    loadCompleted();
    return;
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

  if (taskAdded) {
    TaskScheduler::scheduleTask(
        new TaskFunction([this]() { loadCompleted(); }, Task::Reschedulable));
  } else {
    loadCompleted();
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
        endRemoveRows();
      } else {
        beginInsertRows(QModelIndex(), pos, pos);
        endInsertRows();
      }
      break;
    }
  });

  emit addonCreated(addon);

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

  if (addon) {
    if (addon->enabled()) {
      addon->disable();
    }

    QDir dir;
    if (m_addonDirectory.getDirectory(&dir)) {
      QString addonFileName(QString("%1.rcc").arg(addonId));
      QString addonFilePath(dir.filePath(addonFileName));
      QResource::unregisterResource(addonFilePath, mountPath(addonId));
    }

    addon->deleteLater();
  }

  m_addons.remove(addonId);
  emit countChanged();
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

  emit countChanged();
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

int AddonManager::rowCount(const QModelIndex&) const { return count(); }

int AddonManager::count() const {
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

  // Group containing all the message settings.
  // It only needs to live for the scope of this function.
  SettingGroup* messageSettingGroup =
      SettingsManager::instance()->createSettingGroup(
          ADDON_MESSAGE_SETTINGS_GROUP);
  messageSettingGroup->remove();
}

// static
QString AddonManager::mountPath(const QString& addonId) {
  return QString("/addons/%1").arg(addonId);
}

void AddonManager::refreshAddons() {
  logger.debug() << "Force an addon refresh";
  TaskScheduler::scheduleTask(new TaskAddonIndex());
}

void AddonManager::reset() {
  m_addonDirectory.reset();

  QStringList addonIds;
  for (QMap<QString, AddonData>::const_iterator i(m_addons.constBegin());
       i != m_addons.constEnd(); ++i) {
    addonIds.append(i.key());
  }

  for (const QString& addonId : addonIds) {
    unload(addonId);
    removeAddon(addonId);
  }

  refreshAddons();
}

// static
bool AddonManager::signatureVerificationNeeded() {
  if (Constants::inProduction()) {
    return true;
  }

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains(MVPN_ENV_SKIP_ADDON_SIGNATURE) &&
      !pe.value(MVPN_ENV_SKIP_ADDON_SIGNATURE).isEmpty()) {
    return false;
  }

  return Feature::get(Feature::Feature_addonSignature)->isSupported();
}

void AddonManager::loadCompleted() {
  m_loadCompleted = true;
  emit loadCompletedChanged();
}
