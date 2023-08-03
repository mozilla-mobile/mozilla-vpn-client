/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorwebchannel.h"

#include <QApplication>
#include <QHostAddress>
#include <QQmlApplicationEngine>
#include <QWebSocket>

#include "inspectorwebsockettransport.h"
#include "qmlengineholder.h"
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
InspectorWebChannel::~InspectorWebChannel() {}

void InspectorWebChannel::attach(QWebSocket* target){
    m_channel.connectTo(new InspectorWebSocketTransport(target));
}

InspectorWebChannel::InspectorWebChannel(QObject* parent) : QObject(parent) {
    // Setup the Typeexport
    m_channel.registerObject("settings",SettingsHolder::instance());

    auto engineHolder = QmlEngineHolder::instance();
    QQmlApplicationEngine* engine =
        static_cast<QQmlApplicationEngine*>(engineHolder->engine());

    m_channel.registerObject("engine", engine->rootObjects().first());
}
