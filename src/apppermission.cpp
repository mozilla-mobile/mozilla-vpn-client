/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apppermission.h"
#include "applistprovider.h"
#include "logger.h"
#include "leakdetector.h"
#include "mozillavpn.h"
#include <QApplication>
#include <QVector>
#include "settingsholder.h"
#include "l18nstrings.h"

#if defined(MVPN_ANDROID)
#  include "platforms/android/androidapplistprovider.h"
#elif defined(MVPN_LINUX)
#  include "platforms/linux/linuxapplistprovider.h"
#elif defined(MVPN_WINDOWS)
#  include "platforms/windows/windowsapplistprovider.h"
#else
#  include "platforms/dummy/dummyapplistprovider.h"
#endif

#include <QFileDialog>

namespace {
Logger logger(LOG_MAIN, "AppPermission");
AppPermission* s_instance = nullptr;
}  // namespace

AppPermission::AppPermission(QObject* parent) : QAbstractListModel(parent) {
  MVPN_COUNT_CTOR(AppPermission);
  Q_ASSERT(!s_instance);
  s_instance = this;

  m_listprovider =
#if defined(MVPN_ANDROID)
      new AndroidAppListProvider(this);
#elif defined(MVPN_LINUX)
      new LinuxAppListProvider(this);
#elif defined(MVPN_WINDOWS)
      new WindowsAppListProvider(this);
#else
      new DummyAppListProvider(this);
#endif

  connect(m_listprovider, &AppListProvider::newAppList, this,
          &AppPermission::receiveAppList);
}
AppPermission::~AppPermission() {
  MVPN_COUNT_DTOR(AppPermission);
  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

AppPermission* AppPermission::instance() {
  if (s_instance == nullptr) {
    new AppPermission(qApp);
  }
  Q_ASSERT(s_instance);
  return s_instance;
}

QHash<int, QByteArray> AppPermission::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[AppNameRole] = "appName";
  roles[AppIdRole] = "appID";
  roles[AppEnabledRole] = "appIsEnabled";
  return roles;
}
int AppPermission::rowCount(const QModelIndex&) const {
  return m_applist.size();
}
QVariant AppPermission::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  const AppDescription& app = m_applist.at(index.row());

  switch (role) {
    case AppNameRole:
      return app.name;
    case AppIdRole:
      return QVariant(app.id);
    case AppEnabledRole:
      if (!SettingsHolder::instance()->hasVpnDisabledApps()) {
        // All are enabled then
        return true;
      }
      return !SettingsHolder::instance()->vpnDisabledApps().contains(app.id);
    default:
      return QVariant();
  }
}

void AppPermission::flip(const QString& appID) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  if (settingsHolder->hasVpnDisabledApp(appID)) {
    logger.debug() << "Enabled --" << appID << " for VPN";
    settingsHolder->removeVpnDisabledApp(appID);

  } else {
    logger.debug() << "Disabled --" << appID << " for VPN";
    settingsHolder->addVpnDisabledApp(appID);
  }

  int index = m_applist.indexOf(AppDescription(appID));
  dataChanged(createIndex(index, 0), createIndex(index, 0));
}

void AppPermission::requestApplist() {
  logger.debug() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(const QMap<QString, QString>& applist) {
  QMap<QString, QString> applistCopy = applist;
  QList<QString> removedMissingApps;
  // Add Missing apps, cleanup ones that we can't find anymore.
  // If that happens
  if (SettingsHolder::instance()->hasMissingApps()) {
    auto missingAppList = SettingsHolder::instance()->missingApps();
    for (const auto& appPath : missingAppList) {
      // Check if the App Still exists, otherwise clean up.
      if (!m_listprovider->isValidAppId(appPath)) {
        SettingsHolder::instance()->removeMissingApp(appPath);
        removedMissingApps.append(m_listprovider->getAppName(appPath));
      } else {
        applistCopy.insert(appPath, m_listprovider->getAppName(appPath));
      }
    }
  }

  auto keys = applistCopy.keys();
  if (!m_applist.isEmpty()) {
    // Check the Disabled-List
    SettingsHolder* settingsHolder = SettingsHolder::instance();
    foreach (QString blockedAppId, settingsHolder->vpnDisabledApps()) {
      if (!m_listprovider->isValidAppId(blockedAppId)) {
        // In case the AppID is no longer valid we don't need to keep it
        logger.debug() << "Removed obsolete appid" << blockedAppId;
        settingsHolder->removeVpnDisabledApp(blockedAppId);
        removedMissingApps.append(m_listprovider->getAppName(blockedAppId));
      } else if (!keys.contains(blockedAppId)) {
        // In case the AppID is valid but not in our applist, we need to create
        // an entry
        logger.debug() << "Added missing appid" << blockedAppId;
        m_applist.append(
            AppDescription(blockedAppId, applistCopy[blockedAppId]));
      }
    }
  }
  beginResetModel();
  logger.debug() << "Recived new Applist -- Entrys: " << applistCopy.size();
  m_applist.clear();
  for (auto id : keys) {
    m_applist.append(AppDescription(id, applistCopy[id]));
  }
  std::sort(m_applist.begin(), m_applist.end());
  endResetModel();

  // In Case we removed Missing Apps during cleanup,
  // Notify the user
  if (removedMissingApps.isEmpty()) {
    return;
  }
  auto strings = L18nStrings::instance();
  QString action = strings->t(L18nStrings::SplittunnelMissingAppActionButton);

  QString message = strings->t(L18nStrings::SplittunnelMissingAppMultiple)
                        .arg(removedMissingApps.count());
  emit notification("warning", message, action);
}

void AppPermission::protectAll() {
  logger.debug() << "Protected all";

  SettingsHolder::instance()->setVpnDisabledApps(QStringList());
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
};

void AppPermission::unprotectAll() {
  logger.debug() << "Unprotected all";

  QStringList allAppIds;
  for (auto app : m_applist) {
    allAppIds.append(app.id);
  }
  SettingsHolder::instance()->setVpnDisabledApps(allAppIds);
  dataChanged(createIndex(0, 0), createIndex(m_applist.size(), 0));
}

void AppPermission::openFilePicker() {
  logger.debug() << "File picker required";

  QFileDialog fp(nullptr, qtTrId("vpn.protectSelectedApps.addApplication"));

  QStringList locations =
      QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
  if (!locations.isEmpty()) {
    fp.setDirectory(locations.first());
  }

  fp.setFilter(QDir::Executable | QDir::Files | QDir::NoDotAndDotDot);
  fp.setFileMode(QFileDialog::ExistingFile);

  if (!fp.exec()) {
    logger.error() << "File picker exection aborted";
    return;
  }

  QStringList fileNames = fp.selectedFiles();
  if (fileNames.isEmpty()) {
    logger.warning() << "File picker - no selection";
    return;
  }

  logger.debug() << "Selection:" << fileNames;
  Q_ASSERT(fileNames.length() == 1);

  Q_ASSERT(m_listprovider);
  if (!m_listprovider->isValidAppId(fileNames[0])) {
    logger.debug() << "App not valid:" << fileNames[0];
    return;
  }
  if (m_applist.contains(AppDescription(fileNames[0]))) {
    // Already have that app in the list.
    return;
  }
  m_listprovider->addApplication(fileNames[0]);

  QString message = L18nStrings::instance()
                        ->t(L18nStrings::SplittunnelMissingAppAddedOne)
                        .arg(m_listprovider->getAppName(fileNames[0]));
  emit notification("success", message);
}
