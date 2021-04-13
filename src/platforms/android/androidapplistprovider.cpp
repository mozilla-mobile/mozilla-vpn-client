/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidapplistprovider.h"
#include "leakdetector.h"

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>
#include <QJsonDocument>
#include <QJsonObject>
#include <jni.h>

#include "logger.h"
#include "leakdetector.h"

namespace {
Logger logger(LOG_CONTROLLER, "AndroidAppListProvider");
}

AndroidAppListProvider::AndroidAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MVPN_COUNT_CTOR(AndroidAppListProvider);
}

void AndroidAppListProvider::getApplicationList() {
  logger.log() << "Fetch Application list from Android";

  QAndroidJniObject activity = QtAndroid::androidActivity();
  Q_ASSERT(activity.isValid());

  QAndroidJniObject str = QAndroidJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/PackageManagerHelper", "getAllAppNames",
      "(Landroid/content/Context;)Ljava/lang/String;", activity.object());
  QJsonDocument appList = QJsonDocument::fromJson(str.toString().toLocal8Bit());
  QJsonObject listObj = appList.object();
  QStringList keys = listObj.keys();

  QMap<QString, QString> out;
  foreach (auto key, keys) { out[key] = listObj[key].toString(); }

  emit newAppList(out);
}
