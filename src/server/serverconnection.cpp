/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverconnection.h"
#include "core.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"

#include <functional>

#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>

constexpr uint32_t MAX_MSG_SIZE = 1024 * 1024;

namespace {

Logger logger(LOG_SERVER, "ServerConnection");

struct RequestType {
  QString m_name;
  std::function<QJsonObject(const QJsonObject&)> m_callback;
};

static QList<RequestType> s_types{
    RequestType{"activate",
                [](const QJsonObject&) {
                  Core::instance()->activate();
                  return QJsonObject();
                }},

    RequestType{"deactivate",
                [](const QJsonObject&) {
                  Core::instance()->deactivate();
                  return QJsonObject();
                }},

    RequestType{"servers",
                [](const QJsonObject&) {
                  QByteArray serverJson =
                      Core::instance()->serverCountryModel()->rawJson();
                  if (serverJson.isEmpty()) {
                    return QJsonObject();
                  }

                  QJsonDocument doc = QJsonDocument::fromJson(serverJson);
                  Q_ASSERT(doc.isObject());

                  QJsonObject obj;

                  // TODO: remove some of the unused fields
                  obj["servers"] = doc.object();
                  return obj;
                }},
};

}  // namespace

ServerConnection::ServerConnection(QObject* parent, QTcpSocket* connection)
    : QObject(parent), m_connection(connection) {
  MVPN_COUNT_CTOR(ServerConnection);

#if !defined(MVPN_ANDROID) && !defined(MVPN_IOS)
  // `::ffff:127.0.0.1` is the IPv4 localhost address written with the IPv6
  // notation.
  Q_ASSERT(connection->localAddress() == QHostAddress("::ffff:127.0.0.1") ||
           connection->localAddress() == QHostAddress::LocalHost ||
           connection->localAddress() == QHostAddress::LocalHostIPv6);
#endif

  logger.debug() << "New connection received";

  Q_ASSERT(m_connection);
  connect(m_connection, &QTcpSocket::readyRead, this,
          &ServerConnection::readData);

  Core* core = Core::instance();
  Q_ASSERT(core);

  connect(core, &Core::stateChanged, this, &ServerConnection::writeState);
  connect(core->controller(), &Controller::stateChanged, this,
          &ServerConnection::writeState);
}

ServerConnection::~ServerConnection() {
  MVPN_COUNT_DTOR(ServerConnection);
  logger.debug() << "Connection released";
}

void ServerConnection::readData() {
  QByteArray input = m_connection->readAll();
  m_buffer.append(input);

  while (true) {
    switch (m_state) {
      case ReadingLength: {
        if (m_buffer.length() < 4) {
          return;
        }

        QByteArray messageLength = m_buffer.left(4);
        m_buffer.remove(0, 4);

        m_messageLength =
            *reinterpret_cast<const uint32_t*>(messageLength.constData());

        if (!m_messageLength || m_messageLength > MAX_MSG_SIZE) {
          m_connection->close();
          return;
        }

        m_state = ReadingBody;

        [[fallthrough]];
      }

      case ReadingBody: {
        if (m_buffer.length() < (int)m_messageLength) {
          return;
        }

        QByteArray message = m_buffer.left(m_messageLength);
        m_buffer.remove(0, m_messageLength);

        processMessage(message);

        m_messageLength = 0;
        m_state = ReadingLength;
        break;
      }

      default:
        Q_ASSERT(false);
        break;
    }
  }
}

void ServerConnection::writeData(const QByteArray& data) {
  uint32_t length = (uint32_t)data.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  if (m_connection->write(rawLength, sizeof(uint32_t)) != sizeof(uint32_t) ||
      m_connection->write(data.constData(), length) != length) {
    m_connection->close();
  }
}

void ServerConnection::writeState() {
  Core* core = Core::instance();
  Q_ASSERT(core);

  QJsonObject obj;
  obj["t"] = "status";

  {
    QString stateStr;
    switch (core->state()) {
      case Core::StateInitialize:
        stateStr = "initialize";
        break;
      case Core::StateAuthenticating:
        stateStr = "authenticating";
        break;
      case Core::StatePostAuthentication:
        [[fallthrough]];
      case Core::StateTelemetryPolicy:
        [[fallthrough]];
      case Core::StateMain:
        stateStr = "ready";
        break;
      case Core::StateUpdateRequired:
        stateStr = "updateRequired";
        break;
      case Core::StateSubscriptionNeeded:
        stateStr = "subscriptionNeeded";
        break;
      case Core::StateSubscriptionInProgress:
        stateStr = "subscriptionInProgress";
        break;
      case Core::StateSubscriptionBlocked:
        stateStr = "subscriptionBlocked";
        break;
      case Core::StateDeviceLimit:
        stateStr = "deviceLimit";
        break;
      case Core::StateBackendFailure:
        stateStr = "backendFailure";
        break;
      case Core::StateBillingNotAvailable:
        stateStr = "billingNotAvailable";
        break;
      case Core::StateSubscriptionNotValidated:
        stateStr = "subscriptionNotValidated";
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    obj["app"] = stateStr;
  }

  {
    QString controllerStateStr;
    switch (core->controller()->state()) {
      case Controller::StateInitializing:
        controllerStateStr = "initializing";
        break;
      case Controller::StateOff:
        controllerStateStr = "off";
        break;
      case Controller::StateConnecting:
        controllerStateStr = "connecting";
        break;
      case Controller::StateConfirming:
        controllerStateStr = "confirming";
        break;
      case Controller::StateOn:
        controllerStateStr = "on";
        break;
      case Controller::StateDisconnecting:
        controllerStateStr = "disconnecting";
        break;
      case Controller::StateSwitching:
        controllerStateStr = "switching";
        break;
      default:
        Q_ASSERT(false);
        break;
    }

    obj["vpn"] = controllerStateStr;
  }

  writeData(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void ServerConnection::writeInvalidRequest() {
  QJsonObject obj;
  obj["t"] = "invalidRequest";
  writeData(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void ServerConnection::processMessage(const QByteArray& message) {
  QJsonDocument json = QJsonDocument::fromJson(message);
  if (!json.isObject()) {
    writeInvalidRequest();
    return;
  }

  QJsonObject obj = json.object();
  QString typeName = obj["t"].toString();

  for (const RequestType& type : s_types) {
    if (typeName == type.m_name) {
      QJsonObject responseObj = type.m_callback(obj);
      responseObj["t"] = typeName;
      writeData(QJsonDocument(responseObj).toJson(QJsonDocument::Compact));
      return;
    }
  }

  writeInvalidRequest();
}
