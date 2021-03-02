/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidnetworkwatcher.h"
#include "leakdetector.h"
#include "logger.h"
#include <QtAndroid>
#include <jni.h>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

namespace {
Logger logger(LOG_ANDROID, "AndroidNetworkWatcher");
AndroidNetworkWatcher* s_instance = nullptr;
}  // namespace

AndroidNetworkWatcher::AndroidNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(AndroidNetworkWatcher);
  Q_ASSERT(s_instance == nullptr);
  s_instance = this;
}

AndroidNetworkWatcher::~AndroidNetworkWatcher() {
  MVPN_COUNT_DTOR(AndroidNetworkWatcher);
  s_instance = nullptr;
}

void AndroidNetworkWatcher::initialize() {
  logger.log() << "initialize";
  // Hook in the native implementation for startActivityForResult into the JNI
  JNINativeMethod methods[]{
      {"insecureNetworkDetected", "(Ljava/lang/String;Ljava/lang/String;)V",
       reinterpret_cast<void*>(onInsecureNetworkDetected)}};
  QAndroidJniObject javaClass("org/mozilla/firefox/vpn/NetworkChangeReceiver");

  QAndroidJniEnvironment env;
  jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
  env->RegisterNatives(objectClass, methods,
                       sizeof(methods) / sizeof(methods[0]));
  env->DeleteLocalRef(objectClass);

  QAndroidJniObject::callStaticMethod<void>(
      "org/mozilla/firefox/vpn/NetworkChangeReceiver", "registerIntentFilter",
      "(Landroid/content/Context;)V", QtAndroid::androidActivity().object());
  logger.log() << "callback registered";
}

// java-external static
void AndroidNetworkWatcher::onInsecureNetworkDetected(jstring ssid,
                                                      jstring bssid) {
  logger.log() << "Network changed callback";
  if (!s_instance || !s_instance->isActive()) {
    return;
  }
  QAndroidJniEnvironment env;
  QString qssid(env->GetStringUTFChars(ssid, 0));
  QString qbssid(env->GetStringUTFChars(bssid, 0));
  emit s_instance->unsecuredNetwork(qssid, qbssid);
}
