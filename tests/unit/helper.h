/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPER_H
#define HELPER_H

#include <QObject>
#include <QVector>
#include <QtTest/QtTest>

#include "connectionmanager.h"
#include "controller.h"
#include "mozillavpn.h"
#include "notificationhandler.h"

class NetworkRequest;

class TestHelper : public QObject {
  Q_OBJECT

 public:
  TestHelper();

 public:
  struct NetworkConfig {
    enum NetworkStatus {
      Success,
      Failure,
    };
    NetworkStatus m_status;
    QByteArray m_body;

    NetworkConfig(NetworkStatus status, const QByteArray& body)
        : m_status(status), m_body(body) {}
  };

  static bool networkRequestDelete(NetworkRequest* request) {
    return networkRequestGeneric(request);
  }

  static bool networkRequestGet(NetworkRequest* request) {
    return networkRequestGeneric(request);
  }

  static bool networkRequestPost(NetworkRequest* request,
                                 const QByteArray& data) {
    return networkRequestGeneric(request);
  }

  static bool networkRequestPostIODevice(NetworkRequest* request,
                                         QIODevice* device) {
    return networkRequestGeneric(request);
  }

  static bool networkRequestGeneric(NetworkRequest* request);

  static QVector<NetworkConfig> networkConfig;

  static ConnectionManager::State controllerState;

  struct SystemNotification {
    NotificationHandler::Message type;
    QString title;
    QString message;
    int timer;
  };

  static SystemNotification lastSystemNotification;

  static void resetLastSystemNotification() {
    TestHelper::SystemNotification notification;
    notification.title = QString();
    notification.message = QString();
    TestHelper::lastSystemNotification = notification;
  }

  static QVector<QObject*> testList;

  static QObject* findTest(const QString& name);
};

#endif  // HELPER_H
