/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidvpnactivity.h"

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "androidutils.h"
#include "constants.h"
#include "frontend/navigator.h"
#include "jni.h"
#include "logger.h"
#include "logoutobserver.h"
#include "mozillavpn.h"
#include "platforms/android/androidcommons.h"
#include "settingsholder.h"

namespace {
AndroidVPNActivity* s_instance = nullptr;
constexpr auto CLASSNAME = "org.mozilla.firefox.vpn.qt.VPNActivity";
Logger logger("AndroidVPNActivity");
}  // namespace

AndroidVPNActivity::AndroidVPNActivity() {
  AndroidCommons::runOnAndroidThreadSync([]() {
    // Hook in the native implementation for startActivityForResult into the JNI
    JNINativeMethod methods[]{
        {"handleBackButton", "()Z", reinterpret_cast<bool*>(handleBackButton)},
        {"onServiceMessage", "(ILjava/lang/String;)V",
         reinterpret_cast<void*>(onServiceMessage)},
        {"qtOnServiceConnected", "()V",
         reinterpret_cast<void*>(onServiceConnected)},
        {"qtOnServiceDisconnected", "()V",
         reinterpret_cast<void*>(onServiceDisconnected)},
        {"onIntentInternal", "()V", reinterpret_cast<void*>(onIntentInternal)}};
    QJniObject javaClass(CLASSNAME);
    QJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
    logger.debug() << "Registered native methods";
    QJniObject::callStaticMethod<void>(CLASSNAME, "nativeMethodsRegistered",
                                       "()V");
  });

  QObject::connect(SettingsHolder::instance(),
                   &SettingsHolder::startAtBootChanged, this,
                   &AndroidVPNActivity::startAtBootChanged);

  LogoutObserver* lo = new LogoutObserver(this);
  QObject::connect(lo, &LogoutObserver::ready, this,
                   &AndroidVPNActivity::onLogout);

  QObject::connect(MozillaVPN::instance(), &MozillaVPN::stateChanged, this,
                   &AndroidVPNActivity::onAppStateChange);
}

void AndroidVPNActivity::maybeInit() {
  if (s_instance == nullptr) {
    s_instance = new AndroidVPNActivity();
  }
}

// static
bool AndroidVPNActivity::handleBackButton(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  return Navigator::instance()->eventHandled();
}

void AndroidVPNActivity::connectService() {
  QJniObject::callStaticMethod<void>(CLASSNAME, "connectService", "()V");
  logger.debug() << "attempt to connect service";
}

// static
AndroidVPNActivity* AndroidVPNActivity::instance() {
  if (s_instance == nullptr) {
    AndroidVPNActivity::maybeInit();
  }
  return s_instance;
}

// static
void AndroidVPNActivity::sendToService(ServiceAction type,
                                       const QString& data) {
  int messageType = (int)type;
  if (!Constants::inProduction()) {
    logger.debug() << "sendToService: " << messageType << " " << data;
  }
  QJniEnvironment env;
  QJniObject::callStaticMethod<void>(
      CLASSNAME, "sendToService", "(ILjava/lang/String;)V",
      static_cast<int>(messageType),
      QJniObject::fromString(data).object<jstring>());
}

// static
void AndroidVPNActivity::onServiceMessage(JNIEnv* env, jobject thiz,
                                          jint messageType, jstring body) {
  Q_UNUSED(thiz);
  const char* buffer = env->GetStringUTFChars(body, nullptr);
  if (!buffer) {
    return;
  }
  QString parcelBody(buffer);
  env->ReleaseStringUTFChars(body, buffer);
  AndroidCommons::dispatchToMainThread([messageType, parcelBody] {
    AndroidVPNActivity::instance()->handleServiceMessage(messageType,
                                                         parcelBody);
  });
}

void AndroidVPNActivity::handleServiceMessage(int code, const QString& data) {
  auto mode = (ServiceEvents)code;
  switch (mode) {
    case ServiceEvents::EVENT_INIT:
      emit eventInitialized(data);
      break;
    case ServiceEvents::EVENT_CONNECTED:
      emit eventConnected(data);
      break;
    case ServiceEvents::EVENT_DISCONNECTED:
      emit eventDisconnected(data);
      break;
    case ServiceEvents::EVENT_STATISTIC_UPDATE:
      emit eventStatisticUpdate(data);
      break;
    case ServiceEvents::EVENT_ACTIVATION_ERROR:
      emit eventActivationError(data);
      break;
    case ServiceEvents::EVENT_REQUEST_GLEAN_UPLOAD_ENABLED:
      emit eventRequestGleanUploadEnabledState();
      break;
    case ServiceEvents::EVENT_REQUEST_NOTIFICATION_PERMISSION:
      // This is completely handled in Java-Land
      // See VPNActivity.java
      break;
    case ServiceEvents::EVENT_ONBOARDING_COMPLETED:
      emit eventOnboardingCompleted();
      break;
    case ServiceEvents::EVENT_VPN_CONFIG_PERMISSION_RESPONSE:
      emit eventVpnConfigPermissionResponse(data == "granted");
      break;
    default:
      Q_ASSERT(false);
  }
}

void AndroidVPNActivity::onServiceConnected(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  logger.debug() << "service connected";
  emit AndroidVPNActivity::instance()->serviceConnected();
}

void AndroidVPNActivity::onServiceDisconnected(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  logger.debug() << "service disconnected";
  emit AndroidVPNActivity::instance()->serviceDisconnected();
}
void AndroidVPNActivity::startAtBootChanged() {
  QJsonObject args;
  args["startOnBoot"] = SettingsHolder::instance()->startAtBoot();
  QJsonDocument doc(args);
  sendToService(ServiceAction::ACTION_SET_START_ON_BOOT, doc.toJson());
}

void AndroidVPNActivity::onLogout() {
  sendToService(ServiceAction::ACTION_CLEAR_STORAGE);
}

void AndroidVPNActivity::onAppStateChange() {
  if (!Constants::inProduction()) {
    // Do not restrict screencap on debug
    return;
  }
  // When the App State changes, check if we are doing Authentication
  // if so, mark the content as sensitive, so no screenshots can be taken from
  // those screens.
  auto state = MozillaVPN::instance()->state();
  bool isSensitive = state == App::StateAuthenticating;
  AndroidCommons::runOnAndroidThreadSync([isSensitive]() {
    QJniObject::callStaticMethod<void>(CLASSNAME, "setScreenSensitivity",
                                       "(Z)V", isSensitive);
  });
}

QUrl AndroidVPNActivity::getOpenerURL() {
  logger.debug() << "Getting deep-link:";
  QJniEnvironment env;
  QJniObject string = QJniObject::callStaticObjectMethod(
      CLASSNAME, "getOpenerURL", "()Ljava/lang/String;");
  jstring value = (jstring)string.object();
  auto buf = AndroidUtils::getQByteArrayFromJString(env.jniEnv(), value);

  QString maybeURL = QString::fromUtf8(buf);
  if (maybeURL.isEmpty()) {
    return QUrl();
  }
  logger.debug() << "Got with deep-link:" << maybeURL;
  QUrl url(maybeURL);
  if (!url.isValid()) {
    return QUrl();
  }
  if (url.scheme() != Constants::DEEP_LINK_SCHEME) {
    return QUrl();
  }
  return url;
}

void AndroidVPNActivity::onIntentInternal(JNIEnv* env, jobject thiz) {
  logger.debug() << "Activity Resumed with a new Intent";
  auto url = getOpenerURL();
  if (url.isEmpty()) {
    return;
  }
  emit s_instance->onOpenedWithUrl(url);
}
