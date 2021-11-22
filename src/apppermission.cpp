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
}  // namespace

AppPermission::AppPermission() : QAbstractListModel(nullptr) {
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

AppPermission* AppPermission::instance() {
  static auto instance = new AppPermission();
  return instance;
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
      if (SettingsHolder::instance()->vpnDisabledApps().isEmpty()) {
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
  QStringList applist = settingsHolder->vpnDisabledApps();
  if (settingsHolder->vpnDisabledApps().contains(appID)) {
    logger.debug() << "Enabled --" << appID << " for VPN";
    applist.removeAll(appID);
  } else {
    logger.debug() << "Disabled --" << appID << " for VPN";
    applist.append(appID);
  }
  settingsHolder->setVpnDisabledApps(applist);

  int index = m_applist.indexOf(AppDescription(appID));
  dataChanged(createIndex(index, 0), createIndex(index, 0));
}

void AppPermission::requestApplist() {
  logger.debug() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(const QMap<QString, QString>& applist) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  QMap<QString, QString> applistCopy = applist;
  QStringList removedMissingApps;

  // Add Missing apps, cleanup ones that we can't find anymore.
  // If that happens

  QStringList missingAppList = settingsHolder->missingApps();
  QMutableStringListIterator iter(missingAppList);
  while (iter.hasNext()) {
    const QString& appPath = iter.next();
    // Check if the App Still exists, otherwise clean up.
    if (!m_listprovider->isValidAppId(appPath)) {
      removedMissingApps.append(m_listprovider->getAppName(appPath));
      iter.remove();
      continue;
    }
    applistCopy.insert(appPath, m_listprovider->getAppName(appPath));
  }

  if (!removedMissingApps.isEmpty()) {
    settingsHolder->setMissingApps(missingAppList);
  }

  auto keys = applistCopy.keys();
  if (!m_applist.isEmpty()) {
    QStringList disabledApps = settingsHolder->vpnDisabledApps();
    // Check the Disabled-List
    QMutableStringListIterator iter(disabledApps);
    while (iter.hasNext()) {
      const QString& blockedAppId = iter.next();

      if (!m_listprovider->isValidAppId(blockedAppId)) {
        // In case the AppID is no longer valid we don't need to keep it
        logger.debug() << "Removed obsolete appid" << blockedAppId;
        removedMissingApps.append(m_listprovider->getAppName(blockedAppId));
        iter.remove();
        continue;
      }

      if (!keys.contains(blockedAppId)) {
        // In case the AppID is valid but not in our applist, we need to
        // create an entry
        logger.debug() << "Added missing appid" << blockedAppId;
        m_applist.append(
            AppDescription(blockedAppId, applistCopy[blockedAppId]));
      }
    }

    settingsHolder->setVpnDisabledApps(disabledApps);
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
