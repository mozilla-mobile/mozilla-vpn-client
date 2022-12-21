/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidapplistprovider.h"

#include <jni.h>

#include <QJniEnvironment>
#include <QJniObject>
#include <QJsonDocument>
#include <QJsonObject>

#include "androidutils.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("AndroidAppListProvider");
}

AndroidAppListProvider::AndroidAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MZ_COUNT_CTOR(AndroidAppListProvider);
}

void AndroidAppListProvider::getApplicationList() {
  logger.debug() << "Fetch Application list from Android";

  QJniObject activity = AndroidUtils::getActivity();
  Q_ASSERT(activity.isValid());

  QJniObject str = QJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "getAllAppNames",
      "(Landroid/content/Context;)Ljava/lang/String;", activity.object());
  QJsonDocument appList = QJsonDocument::fromJson(str.toString().toLocal8Bit());
  QJsonObject listObj = appList.object();
  QStringList keys = listObj.keys();

  QMap<QString, QString> out;
  foreach (auto key, keys) {
    // This comment is to make linter happy.
    out[key] = listObj[key].toString();
  }

  emit newAppList(out);
}
