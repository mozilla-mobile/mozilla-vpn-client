/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummyapplistprovider.h"

#include "leakdetector.h"

DummyAppListProvider::DummyAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MZ_COUNT_CTOR(DummyAppListProvider);
}

DummyAppListProvider::~DummyAppListProvider() {
  MZ_COUNT_DTOR(DummyAppListProvider);
}

void DummyAppListProvider::getApplicationList() {
  QList<AppDescription> appList{{"com.example.one", "a Example App 1", false},
                                {"com.example.two", "B Example App 2", false},
                                {"org.example.one", "c Example App 3", false},
                                {"org.example.two", "D Example App 4", false},
                                {"com.example.a", "e Example App 5", true},
                                {"com.example.b", "F Example App 6", true},
                                {"org.example.c", "g Example App 7", true},
                                {"org.example.d", "H Example App 8", true}};

  emit newAppList(appList);
}
