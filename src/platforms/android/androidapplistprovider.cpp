/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidapplistprovider.h"

#include <jni.h>

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

  QList<AppDescription> out;
  foreach (auto key, keys) {
    // Todo: get for android
    QJsonValue entry = listObj[key];
    auto const obj = entry.toObject();
    out.append(out.size(),
               {key, obj["name"].toString(), obj["isSystemApp"].toBool()});
  }

  emit newAppList(out);
}
