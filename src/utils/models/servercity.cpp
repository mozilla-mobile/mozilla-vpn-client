/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercity.h"

#include <QCoreApplication>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "leakdetector.h"

ServerCity::ServerCity() { MZ_COUNT_CTOR(ServerCity); }

ServerCity::ServerCity(const ServerCity& other) {
  MZ_COUNT_CTOR(ServerCity);
  *this = other;
}

ServerCity& ServerCity::operator=(const ServerCity& other) {
  if (this == &other) return *this;

  m_name = other.m_name;
  m_code = other.m_code;
  m_country = other.m_country;
  m_displayName = other.m_displayName;
  m_latitude = other.m_latitude;
  m_longitude = other.m_longitude;
  m_protocol = other.m_protocol;
  m_servers = other.m_servers;

  return *this;
}

ServerCity::~ServerCity() { MZ_COUNT_DTOR(ServerCity); }

bool ServerCity::fromJson(const QJsonObject& obj, const QString& country) {
  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

  QJsonValue code = obj.value("code");
  if (!code.isString()) {
    return false;
  }

  QJsonValue latitude = obj.value("latitude");
  if (!latitude.isDouble()) {
    return false;
  }

  QJsonValue longitude = obj.value("longitude");
  if (!longitude.isDouble()) {
    return false;
  }

  QJsonValue serversValue = obj.value("servers");
  if (!serversValue.isArray()) {
    return false;
  }

  QList<QString> servers;
  QJsonArray serversArray = serversValue.toArray();
  for (const QJsonValue& serverValue : serversArray) {
    if (!serverValue.isObject()) {
      return false;
    }

    QJsonObject serverObj = serverValue.toObject();
    QJsonValue pubkeyValue = serverObj.value("public_key");
    if (!pubkeyValue.isString()) {
      return false;
    }

    servers.append(pubkeyValue.toString());
  }

  m_name = name.toString();
  m_code = code.toString();
  m_country = country;
  m_hashKey = hashKey(m_country, m_name);
  m_latitude = latitude.toDouble();
  m_longitude = longitude.toDouble();
  m_servers.swap(servers);

  return true;
}

bool ServerCity::fromMasqueJson(const QJsonObject& obj,
                                const QString& country) {
  QJsonValue name = obj.value("name");
  if (!name.isString()) {
    return false;
  }

  QJsonValue code = obj.value("code");
  if (!code.isString()) {
    return false;
  }

  QJsonValue serversValue = obj.value("servers");
  if (!serversValue.isArray()) {
    return false;
  }

  QList<QString> servers;
  QJsonArray serversArray = serversValue.toArray();
  for (const QJsonValue& serverValue : serversArray) {
    if (!serverValue.isObject()) {
      return false;
    }

    // MASQUE servers have no public key, so we key them by hostname.
    QJsonValue hostname = serverValue.toObject().value("hostname");
    if (!hostname.isString()) {
      return false;
    }

    servers.append(hostname.toString());
  }

  m_name = name.toString();
  m_code = code.toString();
  m_country = country;
  m_hashKey = hashKey(m_country, m_name);
  // The identity name (m_name) is the country name to avoid colliding with a
  // WireGuard city of the same name; show the real city (derived from the
  // location code) instead when we know it.
  m_displayName = masqueCityName(m_code);
  // Remote Settings does not carry coordinates for MASQUE locations.
  m_latitude = 0;
  m_longitude = 0;
  m_protocol = Server::ProtocolType::Masque;
  m_servers.swap(servers);

  return true;
}

// static
QString ServerCity::masqueCityName(const QString& code) {
  // MASQUE locations are identified by an IATA/ICAO airport (or datacenter)
  // code. Map the known ones to a real city name. Generic/anycast codes (e.g.
  // "US", "CA", "REC", "Q30") are intentionally absent and fall back to the
  // location's own name.
  static const QHash<QString, QString> cities = {
      {"VIE", "Vienna"},       {"YSBK", "Sydney"},      {"BRU", "Brussels"},
      {"SOF", "Sofia"},        {"SCL", "Santiago"},     {"SKBO", "Bogota"},
      {"MUC", "Munich"},       {"CPH", "Copenhagen"},   {"LETO", "Madrid"},
      {"HEL", "Helsinki"},     {"LFPB", "Paris"},       {"EGLC", "London"},
      {"DUB", "Dublin"},       {"MXP", "Milan"},        {"RJTF", "Tokyo"},
      {"MMQT", "Queretaro"},   {"KUL", "Kuala Lumpur"}, {"EHRD", "Rotterdam"},
      {"OSL", "Oslo"},         {"AKL", "Auckland"},     {"LIS", "Lisbon"},
      {"ESSB", "Stockholm"},   {"WSAT", "Singapore"},   {"BKK", "Bangkok"},
      {"JNB", "Johannesburg"},
  };
  return cities.value(code.toUpper());
}

// static
QString ServerCity::hashKey(const QString& country, const QString cityName) {
  return cityName + "," + country;
}

// static
QString ServerCity::localizedName(const QString& name) {
  return QCoreApplication::translate("ServerCity", qPrintable(name));
}

void ServerCity::setLatency(qint64 msec) {
  m_latency = msec;
  emit latencyChanged();
}

void ServerCity::setConnectionScore(int score) {
  m_connectionScore = score;
  emit scoreChanged();
}
