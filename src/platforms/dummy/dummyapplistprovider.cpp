/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummyapplistprovider.h"

DummyAppListProvider::DummyAppListProvider()
{

}

DummyAppListProvider::~DummyAppListProvider(){

}

void DummyAppListProvider::getApplicationList(){
    QMap<QString,QString> appList;
    appList["com.example.one"] = "Example App 1";
    appList["com.example.two"] = "Example App 2";
    appList["org.example.one"] = "Example App 3";
    appList["org.example.two"] = "Example App 4";
    emit newAppList(appList);
}

