/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidapplistprovider.h"

#include <jni.h>

#include <QHash>
#include <QJniEnvironment>
#include <QJniObject>
#include <QJsonDocument>
#include <QJsonObject>

#include "applistprovider.h"
#include "leakdetector.h"
#include "logger.h"
#include "platforms/android/androidcommons.h"

namespace {
Logger logger("AndroidAppListProvider");
}

AndroidAppListProvider::AndroidAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MZ_COUNT_CTOR(AndroidAppListProvider);
}

void AndroidAppListProvider::getApplicationList() {
  logger.debug() << "Fetch Application list from Android";

  QJniObject activity = AndroidCommons::getActivity();
  Q_ASSERT(activity.isValid());

  QJniObject str = QJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "getAllAppNames",
      "(Landroid/content/Context;)Ljava/lang/String;", activity.object());
  QJsonDocument appList = QJsonDocument::fromJson(str.toString().toLocal8Bit());
  QJsonObject listObj = appList.object();
  QStringList keys = listObj.keys();

  // Count packages per UID: only packages sharing a UID form an app group,
  // as Android cannot exclude them from the VPN independently.
  QHash<int, int> uidCount;
  for (const QString& key : keys) {
    const QJsonObject obj = listObj[key].toObject();
    if (obj.contains("uid")) {
      ++uidCount[obj["uid"].toInt()];
    }
  }

  QList<AppDescription> out;
  foreach (auto key, keys) {
    // Todo: get for android
    QJsonValue entry = listObj[key];
    auto const obj = entry.toObject();
    QString group;
    if (obj.contains("uid") && uidCount.value(obj["uid"].toInt()) > 1) {
      group = QString::number(obj["uid"].toInt());
    }
    out.append(
        {key, obj["name"].toString(), obj["isSystemApp"].toBool(), group});
  }

  emit newAppList(out);
}
