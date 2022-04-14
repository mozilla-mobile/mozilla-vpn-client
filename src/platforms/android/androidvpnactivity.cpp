/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidvpnactivity.h"
#include "androidutils.h"
#include "constants.h"
#include "logger.h"
#include "mozillavpn.h"

#include "jni.h"
#include <QApplication>
#include <QJniObject>
#include <QJniEnvironment>

namespace {
AndroidVPNActivity* s_instance = nullptr;
constexpr auto CLASSNAME = "org.mozilla.firefox.vpn.qt.VPNActivity";
Logger logger(LOG_ANDROID, "AndroidVPNActivity");
}  // namespace

AndroidVPNActivity::AndroidVPNActivity() {
  AndroidUtils::runOnAndroidThreadSync([]() {
    // Hook in the native implementation for startActivityForResult into the JNI
    JNINativeMethod methods[]{
        {"handleBackButton", "()Z", reinterpret_cast<bool*>(handleBackButton)},
        {"onServiceMessage", "(ILjava/lang/String;)V",
         reinterpret_cast<void*>(onServiceMessage)},
        {"qtOnServiceConnected", "()V",
         reinterpret_cast<void*>(onServiceConnected)},
        {"qtOnServiceDisconnected", "()V",
         reinterpret_cast<void*>(onServiceDisconnected)},
    };
    QJniObject javaClass(CLASSNAME);
    QJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);

    logger.debug() << "Registered native methods";
  });
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
  return MozillaVPN::instance()->closeEventHandler()->eventHandled();
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
  AndroidUtils::dispatchToMainThread([messageType, parcelBody] {
    AndroidVPNActivity::instance()->handleServiceMessage(messageType,
                                                         parcelBody);
  });
}

void AndroidVPNActivity::handleServiceMessage(int code, const QString& data) {
  if (code != ServiceEvents::EVENT_BACKEND_LOGS) {
    // Don't put the logs in the log.
    logger.debug() << "handleServiceMessage" << code << data;
  }
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
    case ServiceEvents::EVENT_BACKEND_LOGS:
      emit eventBackendLogs(data);
      break;
    case ServiceEvents::EVENT_ACTIVATION_ERROR:
      emit eventActivationError(data);
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