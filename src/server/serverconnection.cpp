/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverconnection.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "mozillavpn.h"

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
                  MozillaVPN::instance()->activate();
                  return QJsonObject();
                }},

    RequestType{"deactivate",
                [](const QJsonObject&) {
                  MozillaVPN::instance()->deactivate();
                  return QJsonObject();
                }},

    RequestType{"servers",
                [](const QJsonObject&) {
                  QByteArray serverJson =
                      MozillaVPN::instance()->serverCountryModel()->rawJson();
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

  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  connect(vpn, &MozillaVPN::stateChanged, this, &ServerConnection::writeState);
  connect(vpn->controller(), &Controller::stateChanged, this,
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
  MozillaVPN* vpn = MozillaVPN::instance();
  Q_ASSERT(vpn);

  QJsonObject obj;
  obj["t"] = "status";

  {
    QString stateStr;
    switch (vpn->state()) {
      case MozillaVPN::StateInitialize:
        stateStr = "initialize";
        break;
      case MozillaVPN::StateAuthenticating:
        stateStr = "authenticating";
        break;
      case MozillaVPN::StatePostAuthentication:
        [[fallthrough]];
      case MozillaVPN::StateTelemetryPolicy:
        [[fallthrough]];
      case MozillaVPN::StateMain:
        stateStr = "ready";
        break;
      case MozillaVPN::StateUpdateRequired:
        stateStr = "updateRequired";
        break;
      case MozillaVPN::StateSubscriptionNeeded:
        stateStr = "subscriptionNeeded";
        break;
      case MozillaVPN::StateSubscriptionInProgress:
        stateStr = "subscriptionInProgress";
        break;
      case MozillaVPN::StateSubscriptionBlocked:
        stateStr = "subscriptionBlocked";
        break;
      case MozillaVPN::StateDeviceLimit:
        stateStr = "deviceLimit";
        break;
      case MozillaVPN::StateBackendFailure:
        stateStr = "backendFailure";
        break;
      case MozillaVPN::StateBillingNotAvailable:
        stateStr = "billingNotAvailable";
        break;
      case MozillaVPN::StateSubscriptionNotValidated:
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
    switch (vpn->controller()->state()) {
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
