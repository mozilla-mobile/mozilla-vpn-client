/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidnetworkwatcher.h"

#include "platforms/android/androidutils.h"
#include "logger.h"
#include "leakdetector.h"
#include "networkwatcherimpl.h"

#include <QJniObject>
#include <QJniEnvironment>
#include <QApplication>
#include "jni.h"

namespace {
Logger logger(LOG_ANDROID, "AndroidNetworkWatcher");
AndroidNetworkWatcher* s_instance = nullptr;
constexpr auto VPNNetworkWatcher_CLASS =
    "org/mozilla/firefox/vpn/qt/VPNNetworkWatcher";
}  // namespace

AndroidNetworkWatcher::AndroidNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(AndroidNetworkWatcher);
}

AndroidNetworkWatcher::~AndroidNetworkWatcher() {
  MVPN_COUNT_DTOR(AndroidNetworkWatcher);
  s_instance = nullptr;
}

void AndroidNetworkWatcher::initialize() {
  Q_ASSERT(!s_instance);
  s_instance = this;

  // Register our native callback
  AndroidUtils::runOnAndroidThreadSync([]() {
    JNINativeMethod methods[]{
        // Failures
        {"networkChanged", "()V", reinterpret_cast<void*>(onNetworkChange)},
    };
    QJniEnvironment env;
    jclass objectClass = env.findClass(VPNNetworkWatcher_CLASS);
    if (objectClass == nullptr) {
      logger.error() << "Android-Networkwatcher Class is Null?!";
      return;
    }
    env->RegisterNatives(objectClass, methods,
                         sizeof(methods) / sizeof(methods[0]));
  });
  // Register for network changes
  QJniEnvironment env;
  QJniObject activity = AndroidUtils::getActivity();
  QJniObject::callStaticMethod<void>(VPNNetworkWatcher_CLASS, "initListener",
                                     "(Landroid/content/Context;)V",
                                     activity.object());
}

NetworkWatcherImpl::TransportType AndroidNetworkWatcher::getTransportType() {
  QJniEnvironment env;
  QJniObject activity = AndroidUtils::getActivity();
  int type = QJniObject::callStaticMethod<int>(
      VPNNetworkWatcher_CLASS, "getTransportType",
      "(Landroid/content/Context;)I", activity.object());
  return (NetworkWatcherImpl::TransportType)type;
};

// static
void AndroidNetworkWatcher::onNetworkChange(JNIEnv* env, jobject thiz) {
  Q_UNUSED(env);
  Q_UNUSED(thiz);
  if (!s_instance) {
    return;
  }
  emit s_instance->networkStatusChanged();
}