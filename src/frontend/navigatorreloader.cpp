/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "navigatorreloader.h"

#include "navigator.h"

NavigatorReloader::NavigatorReloader(QObject* parent) : QObject(parent) {
  Navigator::instance()->registerReloader(this);
}

NavigatorReloader::~NavigatorReloader() {
  Navigator::instance()->unregisterReloader(this);
}
