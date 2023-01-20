/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "serverconnection.h"

#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QTcpSocket>
#include <functional>

#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "mozillavpn.h"
#include "settingsholder.h"

constexpr uint32_t MAX_MSG_SIZE = 1024 * 1024;

namespace {

Logger logger("ServerConnection");

struct RequestType {
  QString m_name;
  std::function<QJsonObject(const QJsonObject&)> m_callback;
};

void serializeServerCountry(ServerCountryModel* model, QJsonObject& obj) {
  QJsonArray countries;

  for (const ServerCountry& country : model->countries()) {
    QJsonObject countryObj;
    countryObj["name"] = country.name();
    countryObj["code"] = country.code();

    QJsonArray cities;
    for (const ServerCity& city : country.cities()) {
      QJsonObject cityObj;
      cityObj["name"] = city.name();
      cityObj["code"] = city.code();
      cityObj["latitude"] = city.latitude();
      cityObj["longitude"] = city.longitude();

      QJsonArray servers;
      for (const QString& pubkey : city.servers()) {
        const Server& server = model->server(pubkey);
        if (!server.initialized()) {
          continue;
        }

        QJsonObject serverObj;
        serverObj["hostname"] = server.hostname();
        serverObj["ipv4_gateway"] = server.ipv4Gateway();
        serverObj["ipv6_gateway"] = server.ipv6Gateway();
        serverObj["weight"] = (double)server.weight();

        const QString& socksName = server.socksName();
        if (!socksName.isEmpty()) {
          serverObj["socksName"] = socksName;
        }

        uint32_t multihopPort = server.multihopPort();
        if (multihopPort) {
          serverObj["multihopPort"] = (double)multihopPort;
        }

        servers.append(serverObj);
      }

      cityObj["servers"] = servers;
      cities.append(cityObj);
    }

    countryObj["cities"] = cities;
    countries.append(countryObj);
  }

  obj["countries"] = countries;
}

QJsonObject serializeStatus() {
  MozillaVPN* vpn = MozillaVPN::instance();

  QJsonObject locationObj;
  locationObj["exit_country_code"] = vpn->serverData()->exitCountryCode();
  locationObj["exit_city_name"] = vpn->serverData()->exitCityName();
  locationObj["entry_country_code"] = vpn->serverData()->entryCountryCode();
  locationObj["entry_city_name"] = vpn->serverData()->entryCityName();

  QJsonObject obj;
  obj["authenticated"] = vpn->userState() == MozillaVPN::UserAuthenticated;
  obj["location"] = locationObj;

  {
    MozillaVPN::State state = vpn->state();
    const QMetaObject* meta = qt_getEnumMetaObject(state);
    int index = meta->indexOfEnumerator(qt_getEnumName(state));
    obj["app"] = meta->enumerator(index).valueToKey(state);
  }

  {
    Controller::State state = vpn->controller()->state();
    const QMetaObject* meta = qt_getEnumMetaObject(state);
    int index = meta->indexOfEnumerator(qt_getEnumName(state));
    obj["vpn"] = meta->enumerator(index).valueToKey(state);
  }

  return obj;
}

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
                  QJsonObject servers;
                  serializeServerCountry(
                      MozillaVPN::instance()->serverCountryModel(), servers);

                  QJsonObject obj;
                  obj["servers"] = servers;
                  return obj;
                }},

    RequestType{"disabled_apps",
                [](const QJsonObject&) {
                  QJsonArray apps;
                  for (const QString& app :
                       SettingsHolder::instance()->vpnDisabledApps()) {
                    apps.append(app);
                  }

                  QJsonObject obj;
                  obj["disabled_apps"] = apps;
                  return obj;
                }},

    RequestType{"status",
                [](const QJsonObject&) {
                  QJsonObject obj;
                  obj["status"] = serializeStatus();
                  return obj;
                }},
};

}  // namespace

ServerConnection::ServerConnection(QObject* parent, QTcpSocket* connection)
    : QObject(parent), m_connection(connection) {
  MZ_COUNT_CTOR(ServerConnection);

#if !defined(MZ_ANDROID) && !defined(MZ_IOS)
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

  connect(vpn, &MozillaVPN::stateChanged, this, &ServerConnection::writeState);
  connect(vpn->controller(), &Controller::stateChanged, this,
          &ServerConnection::writeState);
}

ServerConnection::~ServerConnection() {
  MZ_COUNT_DTOR(ServerConnection);
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
  QJsonObject obj = serializeStatus();
  obj["t"] = "status";

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
