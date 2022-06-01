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

namespace {
Logger logger(LOG_ANDROID, "AndroidNetworkWatcher");

constexpr auto VPNNetworkWatcher_CLASS =
    "org/mozilla/firefox/vpn/qt/VPNNetworkWatcher";
}  // namespace

AndroidNetworkWatcher::AndroidNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MVPN_COUNT_CTOR(AndroidNetworkWatcher);
}

AndroidNetworkWatcher::~AndroidNetworkWatcher() {
  MVPN_COUNT_DTOR(AndroidNetworkWatcher);
}

void AndroidNetworkWatcher::initialize() {}

NetworkWatcherImpl::TransportType AndroidNetworkWatcher::getTransportType() {
  QJniEnvironment env;
  QJniObject activity = AndroidUtils::getActivity();
  int type = QJniObject::callStaticMethod<int>(
      VPNNetworkWatcher_CLASS, "getTransportType",
      "(Landroid/content/Context;)I", activity.object());
  return (NetworkWatcherImpl::TransportType)type;
};