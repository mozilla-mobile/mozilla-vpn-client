/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidutils.h"
#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"
#include "settingsholder.h"
#include "qmlengineholder.h"
#include "jni.h"

#include <QJniObject>
#include <QJniEnvironment>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookieJar>
#include <QUrlQuery>

namespace {
AndroidUtils* s_instance = nullptr;
Logger logger(LOG_ANDROID, "AndroidUtils");

constexpr auto UTILS_CLASS = "org/mozilla/firefox/vpn/qt/VPNUtils";
}  // namespace

// static
QString AndroidUtils::GetDeviceName() {
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
  MVPN_COUNT_CTOR(AndroidUtils);

  Q_ASSERT(!s_instance);
  s_instance = this;

  QJniEnvironment env;
  jclass javaClass = env.findClass(UTILS_CLASS);

  JNINativeMethod methods[]{
      {"recordGleanEvent", "(Ljava/lang/String;)V",
       reinterpret_cast<void*>(recordGleanEvent)},
      {"recordGleanEventWithExtraKeys",
       "(Ljava/lang/String;Ljava/lang/String;)V",
       reinterpret_cast<void*>(recordGleanEventWithExtraKeys)},
  };

  env->RegisterNatives(javaClass, methods,
                       sizeof(methods) / sizeof(methods[0]));
}

AndroidUtils::~AndroidUtils() {
  MVPN_COUNT_DTOR(AndroidUtils);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
void AndroidUtils::dispatchToMainThread(std::function<void()> callback) {
  QTimer* timer = new QTimer();
  timer->moveToThread(qApp->thread());
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, [=]() {
    callback();
    timer->deleteLater();
  });
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
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
QString AndroidUtils::getQStringFromJString(JNIEnv* env, jstring data) {
  const char* buffer = env->GetStringUTFChars(data, nullptr);
  if (!buffer) {
    logger.error() << "getQStringFromJString - failed to parse data.";
    return QString();
  }
  QString out(buffer);
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

bool AndroidUtils::ShareText(const QString& text) {
  return (bool)QJniObject::callStaticMethod<jboolean>(
      UTILS_CLASS, "sharePlainText", "(Ljava/lang/String;)Z",
      QJniObject::fromString(text).object());
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
  QJniObject activity = getActivity();
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

QJniObject AndroidUtils::getActivity() {
  return QNativeInterface::QAndroidApplication::context();
}

int AndroidUtils::GetSDKVersion() {
  QJniEnvironment env;
  jclass versionClass = env->FindClass("android/os/Build$VERSION");
  jfieldID sdkIntFieldID = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
  int sdk = env->GetStaticIntField(versionClass, sdkIntFieldID);
  return sdk;
}

QString AndroidUtils::GetManufacturer() {
  QJniEnvironment env;
  jclass buildClass = env->FindClass("android/os/Build");
  jfieldID manuFacturerField =
      env->GetStaticFieldID(buildClass, "MANUFACTURER", "Ljava/lang/String;");
  jstring value =
      (jstring)env->GetStaticObjectField(buildClass, manuFacturerField);

  const char* buffer = env->GetStringUTFChars(value, nullptr);
  if (!buffer) {
    logger.error() << "Failed to fetch MANUFACTURER";
    return QByteArray();
  }
  QString res(buffer);
  logger.info() << "MANUFACTURER: " << res;
  env->ReleaseStringUTFChars(value, buffer);
  return res;
}

void AndroidUtils::runOnAndroidThreadSync(
    const std::function<void()> runnable) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread(runnable)
      .waitForFinished();
}

void AndroidUtils::recordGleanEvent(JNIEnv* env, jobject VPNUtils,
                                    jstring event) {
  Q_UNUSED(VPNUtils);
  const char* buffer = env->GetStringUTFChars(event, nullptr);
  if (!buffer) {
    return;
  }
  if (!MozillaVPN::instance()) {
    return;
  }
  QString eventString(buffer);
  logger.info() << "Glean Event via JNI:" << eventString;
  emit MozillaVPN::instance()->recordGleanEvent(eventString);
  env->ReleaseStringUTFChars(event, buffer);
}

void AndroidUtils::recordGleanEventWithExtraKeys(JNIEnv* env, jobject VPNUtils,
                                                 jstring jevent,
                                                 jstring jextras) {
  if (!MozillaVPN::instance()) {
    return;
  }
  Q_UNUSED(VPNUtils);
  auto event = getQStringFromJString(env, jevent);
  QJsonObject extras = getQJsonObjectFromJString(env, jextras);
  logger.info() << "Glean Event via JNI:" << event;
  emit MozillaVPN::instance()->recordGleanEventWithExtraKeys(
      event, extras.toVariantMap());
}

// static
bool AndroidUtils::verifySignature(const QByteArray& publicKey,
                                   const QByteArray& content,
                                   const QByteArray& signature) {
  QJniEnvironment env;
  auto out = (bool)QJniObject::callStaticMethod<jboolean>(
      UTILS_CLASS, "verifyContentSignature", "([B[B[B)Z",
      tojByteArray(publicKey), tojByteArray(content), tojByteArray(signature));
  logger.info() << "Android Signature Response" << out;
  return out;
}
// Static
// Creates a copy of the passed QByteArray in the JVM and passes back a ref
jbyteArray AndroidUtils::tojByteArray(const QByteArray& data) {
  QJniEnvironment env;
  jbyteArray out = env->NewByteArray(data.size());
  env->SetByteArrayRegion(out, 0, data.size(),
                          reinterpret_cast<const jbyte*>(data.constData()));
  return out;
}