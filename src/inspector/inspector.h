/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOR_H
#  define INSPECTOR_H

#include <QObject>
#include <QPointer>
#include <QProperty>
#include <QQmlApplicationEngine>
#include <QCoreApplication>

#include <QObject>
#include <QWebChannel>
#include <QHostAddress>
#include <QWebSocket>


#include "tools/commandhandler.h"
#include "tools/qquickinspector.h"
#include "server/factory.h"
#include "tools/command.h"
#include "tools/hotreloader.h"

/**
 * @brief Service that allow's other Clients to debug it. 
 * 
 */
class Inspector : public QObject {
	Q_OBJECT

 public:
        Inspector(QCoreApplication* parent_app, QQmlApplicationEngine* engine)
            : QObject(parent_app), m_cli(this), m_hotReloader(this, engine) {
			m_app = parent_app;
			m_engine = engine;
            m_server = InspectorServer::Factory::create(this);
			registerInternals();
		}

		~Inspector() { 
			m_server->deleteLater();
		}

		/*
		* Look's up an object in the current QML namespace
		* and add's that to the exported WebChannel list
		**/
		void exportQmlSingleton(QString objName, QString package = "Mozilla.VPN", int major= 1, int minor = 0) {
            int id = qmlTypeId(package.toUtf8().constData(), major, minor, objName.toUtf8().constData());
            auto qObj= m_engine->singletonInstance<QObject*>(id);
            if (qObj == nullptr) {
              return;
            }
            m_channel.registerObject(objName, qObj);
        }

		void exportQmlSingleton(QStringList objNames,
                                QString package = "Mozilla.VPN", int major = 1,
                                int minor = 0){
            for (const auto name : objNames){
              exportQmlSingleton(name, package, major, minor);
			}
		}

        /**
        * Exports a QObject with the Name ${objName}
        * to the Clients
        */
		void exportObject(QString objName, QObject* o){
                  if (o == nullptr) {
                    return;
                  }
            m_channel.registerObject(objName, o);
		}

		/**
         * @brief Register a new command.
         */
        void registerCommand(const InspectorTools::Command& command) {
                  m_cli.registerCommand(command);
        };


		
		Q_SLOT void onConnection(
                    QWebChannelAbstractTransport* connection) {
					m_channel.connectTo(connection);
        };

        QPointer<InspectorTools::CommandHandler> getCommandHolder() {
                                        return &m_cli;
        }

        QPointer<InspectorTools::Hotreloader> getHotReloader() {
          return &m_hotReloader;
        }


       private:

		void registerInternals(){
          m_channel.registerObject("inspector_cli", &m_cli);
                    m_channel.registerObject("inspector_hotReloader", &m_hotReloader);
                    m_channel.registerObject("app", m_app);
                                        m_channel.registerObject(
                                            "inspector_graph",
                        new InspectorTools::QQuickInspector(this, m_engine));
		}


        QPointer<QCoreApplication> m_app;
		QPointer<QQmlApplicationEngine> m_engine;
        QPointer<QObject> m_server;

        InspectorTools::Hotreloader m_hotReloader;
        InspectorTools::CommandHandler m_cli; 
        QWebChannel m_channel;
};
#endif
