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
#include "settingsholder.h"

constexpr auto COMMON_UTILS_CLASS = "org/mozilla/firefox/qt/common/Utils";
constexpr auto VPN_UTILS_CLASS = "org/mozilla/firefox/vpn/qt/VPNUtils";

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
  // Call the static method to get the instance
  QJniObject activityInstance = QJniObject::callStaticObjectMethod(
      "org/mozilla/firefox/vpn/qt/VPNActivity", "getInstance",
      "()Lorg/mozilla/firefox/vpn/qt/VPNActivity;");

  if (!activityInstance.isValid()) {
    qWarning("Call to VPNActivity.getInstance() failed or returned null.");
    return QJniObject();  // Return an invalid QJniObject
  }

  return activityInstance;
}

void AndroidCommons::forcePublishActivity() {
  using QAA = QNativeInterface::QAndroidApplication;
  auto fut = QAA::runOnAndroidMainThread([]() -> QVariant {
    QJniEnvironment env;
    // 1) Get a context
    QJniObject activity = getActivity();

    // 2) Re-publish to Qt (if available in your Qt build)
    // Safe no-op if it's already the same:
    QJniObject::callStaticMethod<void>(
        "org/qtproject/qt/android/QtNative", "setActivity",
        "(Landroid/app/Activity;)V", activity.object<jobject>());
    return true;
  });
  fut.waitForFinished();
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
      VPN_UTILS_CLASS, "sharePlainText", "(Ljava/lang/String;)Z",
      QJniObject::fromString(text).object());
}

// static
void AndroidCommons::initializeGlean(bool isTelemetryEnabled,
                                     const QString& channel) {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);
  QString gleanDebugTag = settingsHolder->gleanDebugTagActive()
                              ? settingsHolder->gleanDebugTag()
                              : "";

  QNativeInterface::QAndroidApplication::runOnAndroidMainThread(
      [isTelemetryEnabled, channel, gleanDebugTag]() {
        QJniObject::callStaticMethod<void>(
            COMMON_UTILS_CLASS, "initializeGlean",
            "(Landroid/content/Context;ZLjava/lang/String;Ljava/lang/String;)V",
            getActivity().object(), (jboolean)isTelemetryEnabled,
            QJniObject::fromString(channel).object(),
            QJniObject::fromString(gleanDebugTag).object());
      });
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

void AndroidCommons::launchPlayStore() {
  auto appActivity = AndroidCommons::getActivity();
  QJniObject::callStaticMethod<void>(COMMON_UTILS_CLASS, "launchPlayStore",
                                     "(Landroid/app/Activity;)V",
                                     appActivity.object());
}

void AndroidCommons::setStatusBarTextColor(bool isLight) {
  QNativeInterface::QAndroidApplication::runOnAndroidMainThread([isLight]() {
    QJniObject window = AndroidCommons::getActivity().callObjectMethod(
        "getWindow", "()Landroid/view/Window;");
    if (isLight) {
      window.callMethod<void>("setStatusBarColor", "(I)V", 0xFFFFFFFF);
    } else {
      window.callMethod<void>("setStatusBarColor", "(I)V", 0xFF000000);
    }
  });

}

bool AndroidCommons::clearPendingJavaException(const char* where) {
  QJniEnvironment env;
  if (!env->ExceptionCheck()) {
    logger.info() << "No pending exception at" << where;
    return false;
  }
  logger.info() << "[JNI] Pending exception at" << where;
  env->ExceptionDescribe();
  env->ExceptionClear();
  return true;
};

// static
void AndroidCommons::runWhenUiViewConstructible(std::function<void()> fn,
                                                int retryMs) {
  using QAA = QNativeInterface::QAndroidApplication;
  auto attempt = [fn = std::move(fn), retryMs]() mutable {
    // Do the probe on the Android UI thread
    QAA::runOnAndroidMainThread([fn = std::move(fn)]() mutable -> QVariant {
      if (clearPendingJavaException("gate-entry(UI)")) return false;

      // 1) Need an Activity/Application context
      QJniObject ctx = QAA::context();
      if (!ctx.isValid()) return false;

      // 2) Actually try to construct a View with that Context
      QJniEnvironment env;
      QJniObject dummyView("android/view/View", "(Landroid/content/Context;)V",
                           ctx.object());
      if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
      }

      if (clearPendingJavaException("gate-exit(UI)")) return false;

      // Success — run the work on the Qt thread
      QMetaObject::invokeMethod(
          qApp, [fn = std::move(fn)]() mutable { fn(); }, Qt::QueuedConnection);
      return true;
    }).then([retryMs, fn = std::move(fn)](QFuture<QVariant> f) mutable {
      const bool ok =
          f.isValid() && f.result().isValid() && f.result().toBool();
      if (!ok) {
        QTimer::singleShot(retryMs, qApp,
                           [fn = std::move(fn), retryMs]() mutable {
                             AndroidCommons::runWhenUiViewConstructible(
                                 std::move(fn), retryMs);
                           });
      }

      // On older android versions w/ 3 button nav (triangle/circle/square), the
      // status bar color must be manually set.
      if (AndroidCommons::getSDKVersion() < 29) {
        QJniObject window = AndroidCommons::getActivity().callObjectMethod(
            "getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("setStatusBarColor", "(I)V", 0xFF000000);
      }
    });
  };
  QTimer::singleShot(0, qApp, attempt);
}
