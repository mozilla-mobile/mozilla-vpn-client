/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "apppermission.h"

#include <QApplication>
#include <QVector>
#include <algorithm>
#include <utility>

#include "applistprovider.h"
#include "collator.h"
#include "i18nstrings.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/dummy/dummyapplistprovider.h"
#include "settingsholder.h"

#if defined(UNIT_TEST)
// Unit tests use the DummyAppListProvider.
#elif defined(MZ_ANDROID)
#  include "platforms/android/androidapplistprovider.h"
#elif defined(MZ_LINUX)
#  include "platforms/linux/linuxapplistprovider.h"
#elif defined(MZ_MACOS)
#  include "platforms/macos/macosapplistprovider.h"
#elif defined(MZ_WINDOWS)
#  include "platforms/windows/windowsapplistprovider.h"
#endif

#include <QFileDialog>
#include <QStandardPaths>

namespace {
using AppDescription = AppListProvider::AppDescription;
Logger logger("AppPermission");
AppPermission* s_instance = nullptr;

bool sortApplicationCallback(const AppDescription& a, const AppDescription& b,
                             Collator* collator) {
  Q_ASSERT(collator);

  SettingsHolder* settingsHolder = SettingsHolder::instance();
  bool appADisabled = settingsHolder->vpnDisabledApps().contains(a.id);
  bool appBDisabled = settingsHolder->vpnDisabledApps().contains(b.id);

  if (appADisabled == appBDisabled) {
    return collator->compare(a.name.toLower(), b.name.toLower()) < 0;
  }

  return appADisabled;
}

// Returns the IDs of all members of app's app group in applist, including
// app itself. If app has no app group, returns just app's ID.
QStringList groupMemberIds(const QList<AppDescription>& applist,
                           const AppDescription& app) {
  QStringList ids{app.id};
  if (app.appGroup.isEmpty()) {
    return ids;
  }
  for (const auto& other : applist) {
    if (other.id != app.id && other.appGroup == app.appGroup) {
      ids.append(other.id);
    }
  }
  return ids;
}

// Same as above, looking up appID in applist first.
QStringList groupMemberIds(const QList<AppDescription>& applist,
                           const QString& appID) {
  auto it = std::find(applist.begin(), applist.end(), appID);
  if (it == applist.end()) {
    return {appID};
  }
  return groupMemberIds(applist, *it);
}

}  // namespace

struct AppPermission::MissingAppList {
  // Apps that were not in the current app list
  // but are valid, so should be added to the list
  QList<AppListProvider::AppDescription> missedApps;
  // Apps that were no longer found
  // should be removed from settings an a notification shown.
  QStringList appsToRemove;
};

AppPermission::AppPermission(AppListProvider* provider, QObject* parent)
    : QAbstractListModel(parent) {
  MZ_COUNT_CTOR(AppPermission);
  Q_ASSERT(!s_instance);
  s_instance = this;

  m_listprovider = provider;
  provider->setParent(this);

  connect(SettingsHolder::instance(), &SettingsHolder::vpnDisabledAppsChanged,
          this, [this]() {
            beginResetModel();
            endResetModel();
          });

  connect(m_listprovider, &AppListProvider::newAppList, this,
          &AppPermission::receiveAppList);
}

AppPermission::~AppPermission() {
  MZ_COUNT_DTOR(AppPermission);
  Q_ASSERT(s_instance = this);
  s_instance = nullptr;
}

// static
AppPermission* AppPermission::instance() {
  if (s_instance == nullptr) {
#if defined(UNIT_TEST)
    s_instance = new AppPermission(new DummyAppListProvider(qApp));
#elif defined(MZ_ANDROID)
    s_instance = new AppPermission(new AndroidAppListProvider(qApp));
#elif defined(MZ_LINUX)
    s_instance = new AppPermission(new LinuxAppListProvider(qApp));
#elif defined(MZ_MACOS)
    s_instance = new AppPermission(new MacOSAppListProvider(qApp));
#elif defined(MZ_WINDOWS)
    s_instance = new AppPermission(new WindowsAppListProvider(qApp));
#else
    s_instance = new AppPermission(new DummyAppListProvider(qApp));
#endif
  }
  Q_ASSERT(s_instance);
  return s_instance;
}

// static
void AppPermission::mock() {
  s_instance = new AppPermission(new DummyAppListProvider(qApp));
}

QHash<int, QByteArray> AppPermission::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[AppNameRole] = "appName";
  roles[AppIdRole] = "appID";
  roles[AppEnabledRole] = "appIsEnabled";
  roles[AppIsSystemAppRole] = "isSystemApp";
  roles[AppGroupNamesRole] = "appGroupNames";
  return roles;
}
int AppPermission::rowCount(const QModelIndex&) const {
  return static_cast<int>(m_applist.size());
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
    case AppIsSystemAppRole:
      return app.isSystemApp;
    case AppEnabledRole: {
      const QStringList disabledApps =
          SettingsHolder::instance()->vpnDisabledApps();
      for (const QString& id : groupMemberIds(m_applist, app)) {
        if (disabledApps.contains(id)) {
          return false;
        }
      }
      return true;
    }
    case AppGroupNamesRole: {
      if (app.appGroup.isEmpty()) {
        return QString();
      }
      QStringList names;
      for (const auto& other : m_applist) {
        if (other.id != app.id && other.appGroup == app.appGroup) {
          names.append(other.name);
        }
      }
      return names.join(", ");
    }
    default:
      return QVariant();
  }
}

int AppPermission::disabledAppCount() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  QStringList applist = settingsHolder->vpnDisabledApps();
  return static_cast<int>(applist.count());
}

void AppPermission::flip(const QString& appID) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  QStringList applist = settingsHolder->vpnDisabledApps();
  // Apps in the same app group are always excluded together.
  const QStringList group = groupMemberIds(m_applist, appID);
  const bool enable = std::any_of(
      group.begin(), group.end(),
      [&applist](const QString& id) { return applist.contains(id); });
  for (const QString& id : group) {
    applist.removeAll(id);
    if (!enable) {
      applist.append(id);
    }
  }
  logger.debug() << (enable ? "Enabled --" : "Disabled --") << group
                 << " for VPN";
  // This resets the model (see the vpnDisabledAppsChanged connection in the
  // constructor), which refreshes every row of the group.
  settingsHolder->setVpnDisabledApps(applist);
}

void AppPermission::requestApplist() {
  logger.debug() << "Request new AppList";
  m_listprovider->getApplicationList();
}

void AppPermission::receiveAppList(
    const QList<AppListProvider::AppDescription>& applist) {
  auto applistCopy = applist;

  auto missedApps = retrieveMissingApps(applistCopy);

  // Add all missed apps to the current app list
  applistCopy.append(missedApps.missedApps);

  // Pre-app-group settings may exclude only some group members: add the rest.
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  const QStringList disabledApps = settingsHolder->vpnDisabledApps();
  QStringList addedApps;
  for (const QString& appID : disabledApps) {
    for (const QString& memberID : groupMemberIds(applistCopy, appID)) {
      if (!disabledApps.contains(memberID) && !addedApps.contains(memberID)) {
        addedApps.append(memberID);
      }
    }
  }
  if (!addedApps.isEmpty()) {
    logger.debug() << "Excluding app group members" << addedApps;
    settingsHolder->setVpnDisabledApps(disabledApps + addedApps);
  }

  beginResetModel();
  logger.debug() << "Received new Applist -- Entries: " << applistCopy.size();
  m_applist = applistCopy;
  Collator collator;
  std::sort(m_applist.begin(), m_applist.end(),
            std::bind(sortApplicationCallback, std::placeholders::_1,
                      std::placeholders::_2, &collator));

  endResetModel();
  emit containsSystemAppsChanged();

  // In Case we removed Missing Apps during cleanup,
  // Notify the user
  if (missedApps.appsToRemove.isEmpty()) {
    return;
  }
  // Remove the no longer Valid APP id's from the settings
  auto settings_missedApps = settingsHolder->missingApps();
  for (const QString& appToRemove : missedApps.appsToRemove) {
    settings_missedApps.removeAll(appToRemove);
  }
  settingsHolder->setMissingApps(std::as_const(settings_missedApps));

  auto settings_splitTunneledApps = settingsHolder->vpnDisabledApps();
  for (const QString& appToRemove : missedApps.appsToRemove) {
    settings_splitTunneledApps.removeAll(appToRemove);
  }
  settingsHolder->setVpnDisabledApps(settings_splitTunneledApps);

  // Notify the user about the removed apps
  auto strings = I18nStrings::instance();
  QString action = strings->t(I18nStrings::SplittunnelMissingAppActionButton);

  QString message = strings->t(I18nStrings::SplittunnelMissingAppMultiple)
                        .arg(missedApps.appsToRemove.count());
  emit notification("warning", message, action);
}

void AppPermission::protectAll() {
  logger.debug() << "Protected all";

  SettingsHolder::instance()->setVpnDisabledApps(QStringList());
  emit dataChanged(createIndex(0, 0),
                   createIndex(static_cast<int>(m_applist.size()), 0));
};

void AppPermission::unprotectAll() {
  logger.debug() << "Unprotected all";

  QStringList allAppIds;
  for (const auto& app : m_applist) {
    allAppIds.append(app.id);
  }
  SettingsHolder::instance()->setVpnDisabledApps(allAppIds);
  emit dataChanged(createIndex(0, 0),
                   createIndex(static_cast<int>(m_applist.size()), 0));
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

  QString message = I18nStrings::instance()
                        ->t(I18nStrings::SplittunnelMissingAppAddedOne)
                        .arg(m_listprovider->getAppName(fileNames[0]));
  emit notification("success", message);
}

bool AppPermission::containsSystemApps() const {
  for (const auto& app : m_applist) {
    if (app.isSystemApp) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Checks the Split-Tunneled Apps from Settings and finds apps that we
 * failed to look up
 *
 * @param alreadyFoundApps - List of all apps currently found
 * @return Returns 2 Lists:
 * missedApps -> Apps that are valid but were not in the argument set and should
 * be added appsToRemove -> Apps that were no longer found and should be removed
 */
AppPermission::MissingAppList AppPermission::retrieveMissingApps(
    QList<AppListProvider::AppDescription> alreadyFoundApps) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  MissingAppList out;
  //
  QStringList disabledApps = settingsHolder->vpnDisabledApps();
  QStringList missingApps = settingsHolder->missingApps();

  auto const checkList = [&out, &alreadyFoundApps, this](QStringList list) {
    for (const auto& appID : list) {
      // Check if the app is still found
      if (alreadyFoundApps.contains(appID)) {
        continue;
      }

      // Now check if it's valid.
      if (m_listprovider->isValidAppId(appID)) {
        AppDescription appDescription{
            appID,
            m_listprovider->getAppName(appID),
        };
        if (!out.missedApps.contains(appID)) {
          out.missedApps.append(appDescription);
        }
        continue;
      }
      if (!out.appsToRemove.contains(appID)) {
        out.appsToRemove.append(appID);
      }
    }
  };

  checkList(disabledApps);
  checkList(missingApps);

  return out;
}
