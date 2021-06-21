/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef HELPER_H
#define HELPER_H

#include "../../src/mozillavpn.h"
#include "../../src/controller.h"

#include <QObject>
#include <QVector>
#include <QtTest/QtTest>

class TestHelper : public QObject {
  Q_OBJECT

 public:
  TestHelper();

 public:
  struct NetworkConfig {
    enum NetworkStatus {
      Invalid,
      Success,
      Failure,
    };
    NetworkStatus m_status;
    QByteArray m_body;

    NetworkConfig() { m_status = Invalid; }
    NetworkConfig(NetworkStatus status, const QByteArray& body)
        : m_status(status), m_body(body) {}
  };

  static QVector<NetworkConfig> networkConfig;

  static MozillaVPN::State vpnState;

  static Controller::State controllerState;

  static QVector<QObject*> testList;
};

#endif  // HELPER_H
