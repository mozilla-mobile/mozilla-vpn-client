/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidcommons.h"

#include <jni.h>

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QTimer>

#include "logger.h"

constexpr auto COMMON_UTILS_CLASS = "org/mozilla/firefox/qt/common/Utils";

// TODO - to remove:
constexpr auto UTILS_CLASS = "org/mozilla/firefox/vpn/qt/VPNUtils";

namespace {
Logger logger("AndroidCommons");

// Creates a copy of the passed QByteArray in the JVM and passes back a ref
jbyteArray tojByteArray(const QByteArray& data) {
  QJniEnvironment env;
  jbyteArray out = env->NewByteArray(data.size());
  env->SetByteArrayRegion(out, 0, data.size(),
                          reinterpret_cast<const jbyte*>(data.constData()));
  return out;
}

}  // namespace

// static
QJniObject AndroidCommons::getActivity() {
  return QNativeInterface::QAndroidApplication::context();
}

// static
int AndroidCommons::getSDKVersion() {
  QJniEnvironment env;
  jclass versionClass = env->FindClass("android/os/Build$VERSION");
  jfieldID sdkIntFieldID = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
  int sdk = env->GetStaticIntField(versionClass, sdkIntFieldID);
  return sdk;
}

// static
bool AndroidCommons::verifySignature(const QByteArray& publicKey,
                                     const QByteArray& content,
                                     const QByteArray& signature) {
  QJniEnvironment env;
  auto out = (bool)QJniObject::callStaticMethod<jboolean>(
      COMMON_UTILS_CLASS, "verifyContentSignature", "([B[B[B)Z",
      tojByteArray(publicKey), tojByteArray(content), tojByteArray(signature));
  logger.info() << "Android Signature Response" << out;
  return out;
}

// static
bool AndroidCommons::shareText(const QString& text) {
  return (bool)QJniObject::callStaticMethod<jboolean>(
      UTILS_CLASS, "sharePlainText",
      "(Landroid/content/Context;Ljava/lang/String;)Z", getActivity().object(),
      QJniObject::fromString(text).object());
}

// static
void AndroidCommons::initializeGlean(bool isTelemetryEnabled,
                                     const QString& channel) {
  runOnAndroidThreadSync([isTelemetryEnabled, channel]() {
    QJniObject::callStaticMethod<void>(
        UTILS_CLASS, "initializeGlean",
        "(Landroid/content/Context;ZLjava/lang/String;)V",
        getActivity().object(), (jboolean)isTelemetryEnabled,
        QJniObject::fromString(channel).object());
  });
}

// static
void AndroidCommons::runOnAndroidThreadSync(
    const std::function<void()> runnable) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread(runnable)
      .waitForFinished();
}

// static
void AndroidCommons::dispatchToMainThread(std::function<void()> callback) {
  QTimer* timer = new QTimer();
  timer->moveToThread(qApp->thread());
  timer->setSingleShot(true);
  QObject::connect(timer, &QTimer::timeout, [=]() {
    callback();
    timer->deleteLater();
  });
  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection);
}

QString AndroidCommons::GetManufacturer() {
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
