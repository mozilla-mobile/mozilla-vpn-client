/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "servercity.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "constants.h"
#include "feature.h"
#include "leakdetector.h"
#include "location.h"
#include "mozillavpn.h"
#include "serveri18n.h"
#include "serverlatency.h"

// Minimum number of redundant servers we expect at a location.
constexpr int SCORE_SERVER_REDUNDANCY_THRESHOLD = 3;

// Latency threshold for excellent connections, set intentionally very low.
constexpr int SCORE_EXCELLENT_LATENCY_THRESHOLD = 30;

ServerCity::ServerCity() { MZ_COUNT_CTOR(ServerCity); }

ServerCity::ServerCity(const ServerCity& other) {
  MZ_COUNT_CTOR(ServerCity);
  *this = other;

  // Changes in the average latency may cause the connection score to change.
  MozillaVPN* vpn = MozillaVPN::instance();
  if (vpn) {
    connect(vpn->serverLatency(), &ServerLatency::progressChanged, this,
            [this] { emit scoreChanged(); });
  }
}

ServerCity& ServerCity::operator=(const ServerCity& other) {
  if (this == &other) return *this;

  m_name = other.m_name;
  m_code = other.m_code;
  m_country = other.m_country;
  m_latitude = other.m_latitude;
  m_longitude = other.m_longitude;
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
  if (!Constants::inProduction() || !name.toString().contains("BETA")) {
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

// static
QString ServerCity::hashKey(const QString& country, const QString cityName) {
  return cityName + "," + country;
}

const QString ServerCity::localizedName() const {
  return ServerI18N::translateCityName(m_country, m_name);
}

int ServerCity::connectionScore() const {
  ServerLatency* serverLatency = MozillaVPN::instance()->serverLatency();
  qint64 now = QDateTime::currentSecsSinceEpoch();
  int score = Poor;
  int activeServerCount = 0;
  uint32_t sumLatencyMsec = 0;
  for (const QString& pubkey : m_servers) {
    if (serverLatency->getCooldown(pubkey) <= now) {
      sumLatencyMsec += serverLatency->getLatency(pubkey);
      activeServerCount++;
    }
  }

  // Ensure there is at least one reachable server.
  if (activeServerCount == 0) {
    return Unavailable;
  }

  // In the unlikely event that the sum of the latencies is zero, then we
  // haven't actually measured anything and have nothing to report.
  if (sumLatencyMsec == 0) {
    return NoData;
  }

  // Increase the score if the location has better than average latency.
  uint32_t cityLatencyMsec = sumLatencyMsec / activeServerCount;
  if (cityLatencyMsec < serverLatency->avgLatency()) {
    score++;
    // Give the location another point if the latency is *very* fast.
    if (cityLatencyMsec < SCORE_EXCELLENT_LATENCY_THRESHOLD) {
      score++;
    }
  }

  // Increase the score if the location has sufficient redundancy.
  if (activeServerCount >= SCORE_SERVER_REDUNDANCY_THRESHOLD) {
    score++;
  }

  // Increase the score for locations in the same country as the user.
  if (m_country == MozillaVPN::instance()->location()->countryCode()) {
    score++;
  }

  if (score > Excellent) {
    score = Excellent;
  }
  return score;
}

unsigned int ServerCity::latency() const {
  ServerLatency* serverLatency = MozillaVPN::instance()->serverLatency();
  qint64 now = QDateTime::currentSecsSinceEpoch();
  int activeServerCount = 0;
  uint32_t sumLatencyMsec = 0;
  for (const QString& pubkey : m_servers) {
    if (serverLatency->getCooldown(pubkey) <= now) {
      sumLatencyMsec += serverLatency->getLatency(pubkey);
      activeServerCount++;
    }
  }

  return (sumLatencyMsec + activeServerCount - 1) / activeServerCount;
}
