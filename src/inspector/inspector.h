/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOR_H
#  define INSPECTOR_H

#include <QObject>
#include <QPointer>
#include <QProperty>
#include <QQmlApplicationEngine>
#include <QApplication>

#include <QObject>
#include <QWebChannel>
#include <QHostAddress>
#include <QWebSocket>


#include "inspectorhotreloader.h"
#include "inspectorserver.h"


/**
 * @brief Service that allow's other Clients to debug it. 
 * 
 */
class Inspector : public QObject {
	Q_OBJECT

 public:
        Inspector(QApplication* parent_app, 
			QQmlApplicationEngine* engine)
		: QObject(parent_app), 
			m_hotReloader(engine) {
			m_app = parent_app;
			m_engine = engine;
            m_server = InspectorServerFactory::create(this);
			registerInternals();
		}

		/*
		* Look's up an object in the current QML namespace
		* and add's that to the exported WebChannel list
		**/
		void exportQmlSingleton(QString objName, QString package = "Mozilla.VPN", int major= 1, int minor = 0) {
            int id = qmlTypeId(package.toUtf8().constData(), major, minor, objName.toUtf8().constData());
            auto qObj= m_engine->singletonInstance<QObject*>(id);
            m_channel.registerObject(objName, qObj);
        }

		void exportObject(QString objName, QObject* o){
            m_channel.registerObject(objName, o);
		}
		
		Q_SLOT void onConnection(
                    QWebChannelAbstractTransport* connection) {
					m_channel.connectTo(connection);
        };

private:

		void registerInternals(){
                   // m_channel.registerObject("hotReloader", &m_hotReloader);
                    m_channel.registerObject("app", m_app);
		}


        QPointer<QApplication> m_app;
		QPointer<QQmlApplicationEngine> m_engine;
        QPointer<QObject> m_server;

		InspectorHotreloader m_hotReloader;
        QWebChannel m_channel;
};
#endif
