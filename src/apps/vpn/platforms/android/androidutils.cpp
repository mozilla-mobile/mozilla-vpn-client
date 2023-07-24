/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidutils.h"

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookieJar>
#include <QUrlQuery>

#include "jni.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "platforms/android/androidcommons.h"
#include "qmlengineholder.h"
#include "settingsholder.h"

namespace {
AndroidUtils* s_instance = nullptr;
Logger logger("AndroidUtils");

constexpr auto UTILS_CLASS = "org/mozilla/firefox/vpn/qt/VPNUtils";
}  // namespace

// static
QString AndroidUtils::getDeviceName() {
  QJniEnvironment env;
  jclass BUILD = env->FindClass("android/os/Build");
  jfieldID model = env->GetStaticFieldID(BUILD, "DEVICE", "Ljava/lang/String;");
  jstring value = (jstring)env->GetStaticObjectField(BUILD, model);
  if (!value) {
    return QString("Android Device");
  }
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    return QString("Android Device");
  }
  QString res(buffer);
  env->ReleaseStringUTFChars(value, buffer);
  return res;
};

bool AndroidUtils::isChromeOSContext() {
  /*
   * If the device name end's with "_cheets" we're
   * we are running in a Chrome OS / Android Emulator
   * Situation.
   */
  return getDeviceName().endsWith("_cheets");
}

// static
QString AndroidUtils::getDeviceModel() {
  QJniEnvironment env;
  jclass BUILD = env->FindClass("android/os/Build");
  jfieldID model = env->GetStaticFieldID(BUILD, "MODEL", "Ljava/lang/String;");
  jstring value = (jstring)env->GetStaticObjectField(BUILD, model);
  if (!value) {
    return QString("Android Device");
  }
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    return QString("Android Device");
  }
  QString res(buffer);
  env->ReleaseStringUTFChars(value, buffer);
  return res;
};

// static
AndroidUtils* AndroidUtils::instance() {
  if (!s_instance) {
    Q_ASSERT(qApp);
    s_instance = new AndroidUtils(qApp);
  }

  return s_instance;
}

AndroidUtils::AndroidUtils(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(AndroidUtils);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

AndroidUtils::~AndroidUtils() {
  MZ_COUNT_DTOR(AndroidUtils);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
QByteArray AndroidUtils::getQByteArrayFromJString(JNIEnv* env, jstring data) {
  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "getQByteArrayFromJString - failed to parse data.";
    return QByteArray();
  }
  QByteArray out(buffer);
  env->ReleaseStringUTFChars(data, buffer);
  return out;
}

// static
QJsonObject AndroidUtils::getQJsonObjectFromJString(JNIEnv* env, jstring data) {
  QByteArray raw(getQByteArrayFromJString(env, data));
  QJsonParseError jsonError;
  QJsonDocument json = QJsonDocument::fromJson(raw, &jsonError);
  if (QJsonParseError::NoError != jsonError.error) {
    logger.error() << "getQJsonObjectFromJstring - error parsing json. Code: "
                   << jsonError.error << "Offset: " << jsonError.offset
                   << "Message: " << jsonError.errorString()
                   << "Data: " << logger.sensitive(raw);
    return QJsonObject();
  }
  if (!json.isObject()) {
    logger.error() << "getQJsonObjectFromJString - object expected.";
    return QJsonObject();
  }
  return json.object();
}

QByteArray AndroidUtils::DeviceId() {
  /*
   * On Android 8.0 (API level 26) and higher versions of the platform,
   * a 64-bit number (expressed as a hexadecimal string),
   * unique to each combination of app-signing key, user, and device.
   * Values of ANDROID_ID are scoped by signing key and user.
   * The value may change if a factory reset is performed on the device or if an
   * APK signing key changes.
   */
  QJniEnvironment env;
  QJniObject activity = AndroidCommons::getActivity();
  QJniObject string = QJniObject::callStaticObjectMethod(
      UTILS_CLASS, "getDeviceID",
      "(Landroid/content/Context;)Ljava/lang/String;", activity.object());
  jstring value = (jstring)string.object();
  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    logger.error() << "Failed to fetch DeviceID";
    return QByteArray();
  }
  QString res(buffer);
  logger.info() << "DeviceID: " << res;
  env->ReleaseStringUTFChars(value, buffer);
  return res.toUtf8();
}

void AndroidUtils::openNotificationSettings() {
  QJniObject::callStaticMethod<void>(UTILS_CLASS, "openNotificationSettings",
                                     "()V");
}
