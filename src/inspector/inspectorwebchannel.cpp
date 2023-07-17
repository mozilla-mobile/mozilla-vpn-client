/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QApplication>
#include <QHostAddress>
#include <QWebSocket>

#include "inspectorwebchannel.h"
#include "settingsholder.h"


namespace{
    InspectorWebChannel* s_instance = nullptr;
}

InspectorWebChannel* InspectorWebChannel::instance(){
    if(!s_instance){
        s_instance = new InspectorWebChannel(qApp);
    }
    return s_instance;
}


void InspectorWebChannel::attach(QWebSocket* target){
    m_channel.connectTo(target);
}


InspectorWebChannel::InspectorWebChannel(QObject* parent){
    // Setup the Typeexport
    m_channel.registerObject("settings",SettingsHolder::instance());

}

