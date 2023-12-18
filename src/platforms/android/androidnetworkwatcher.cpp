/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidnetworkwatcher.h"

#include <QApplication>
#include <QJniEnvironment>
#include <QJniObject>
#include <QNetworkInformation>

#include "leakdetector.h"
#include "logger.h"
#include "networkwatcherimpl.h"
#include "platforms/android/androidcommons.h"

namespace {
Logger logger("AndroidNetworkWatcher");

// constexpr auto VPNNetworkWatcher_CLASS =
//     "org/mozilla/firefox/vpn/qt/VPNNetworkWatcher";
}  // namespace

AndroidNetworkWatcher::AndroidNetworkWatcher(QObject* parent)
    : NetworkWatcherImpl(parent) {
  MZ_COUNT_CTOR(AndroidNetworkWatcher);
}

AndroidNetworkWatcher::~AndroidNetworkWatcher() {
  MZ_COUNT_DTOR(AndroidNetworkWatcher);
}

void AndroidNetworkWatcher::initialize() {}

QNetworkInformation::Reachability AndroidNetworkWatcher::getReachability() {
  return QNetworkInformation::instance()->reachability();
}
