/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testapppermission.h"

#include <memory>

#include "apppermission.h"
#include "settingsholder.h"

namespace {
using AppDescription = AppListProvider::AppDescription;

const QList<AppDescription> s_apps{
    {"com.example.a", "App A", false, "10100"},
    {"com.example.b", "App B", false, "10100"},
    {"com.example.c", "App C", false},
};

std::unique_ptr<AppPermission> createModel() {
  return std::unique_ptr<AppPermission>(AppPermission::instance());
}

QVariant roleFor(AppPermission* model, const QString& appID, int role) {
  for (int row = 0; row < model->rowCount(QModelIndex()); ++row) {
    QModelIndex index = model->index(row, 0);
    if (model->data(index, AppPermission::AppIdRole).toString() == appID) {
      return model->data(index, role);
    }
  }
  return QVariant();
}

QStringList sorted(QStringList list) {
  list.sort();
  return list;
}
}  // namespace

void TestAppPermission::cleanup() { SettingsHolder::testCleanup(); }

void TestAppPermission::flipGroup() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  auto model = createModel();
  model->receiveAppList(s_apps);

  model->flip("com.example.a");
  QCOMPARE(sorted(settingsHolder->vpnDisabledApps()),
           QStringList({"com.example.a", "com.example.b"}));
  QCOMPARE(roleFor(model.get(), "com.example.b", AppPermission::AppEnabledRole)
               .toBool(),
           false);
  QCOMPARE(roleFor(model.get(), "com.example.c", AppPermission::AppEnabledRole)
               .toBool(),
           true);

  // Flipping the other member re-enables the whole group.
  model->flip("com.example.b");
  QVERIFY(settingsHolder->vpnDisabledApps().isEmpty());
  QCOMPARE(roleFor(model.get(), "com.example.a", AppPermission::AppEnabledRole)
               .toBool(),
           true);
}

void TestAppPermission::flipUngrouped() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  auto model = createModel();
  model->receiveAppList(s_apps);

  model->flip("com.example.c");
  QCOMPARE(settingsHolder->vpnDisabledApps(), QStringList({"com.example.c"}));
  model->flip("com.example.c");
  QVERIFY(settingsHolder->vpnDisabledApps().isEmpty());
}

void TestAppPermission::normalizePartialGroup() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  settingsHolder->setVpnDisabledApps({"com.example.b"});

  auto model = createModel();
  model->receiveAppList(s_apps);

  QCOMPARE(sorted(settingsHolder->vpnDisabledApps()),
           QStringList({"com.example.a", "com.example.b"}));
  QCOMPARE(roleFor(model.get(), "com.example.a", AppPermission::AppEnabledRole)
               .toBool(),
           false);

  // A single flip re-enables the whole group.
  model->flip("com.example.a");
  QVERIFY(settingsHolder->vpnDisabledApps().isEmpty());
}

void TestAppPermission::groupNames() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  auto model = createModel();
  model->receiveAppList(s_apps);

  QCOMPARE(
      roleFor(model.get(), "com.example.a", AppPermission::AppGroupNamesRole)
          .toString(),
      QString("App B"));
  QCOMPARE(
      roleFor(model.get(), "com.example.b", AppPermission::AppGroupNamesRole)
          .toString(),
      QString("App A"));
  QVERIFY(
      roleFor(model.get(), "com.example.c", AppPermission::AppGroupNamesRole)
          .toString()
          .isEmpty());
}

static TestAppPermission s_testAppPermission;
