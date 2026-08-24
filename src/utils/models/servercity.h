/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCITY_H
#define SERVERCITY_H

#include <QList>
#include <QObject>
#include <QString>

#include "server.h"

class QJsonObject;

class ServerCity final : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString code READ code CONSTANT)
  Q_PROPERTY(QString country READ country CONSTANT)
  Q_PROPERTY(QString localizedName READ localizedName CONSTANT)
  Q_PROPERTY(double latitude READ latitude CONSTANT)
  Q_PROPERTY(double longitude READ longitude CONSTANT)
  Q_PROPERTY(qint64 latency READ latency NOTIFY latencyChanged)
  Q_PROPERTY(int connectionScore READ connectionScore NOTIFY scoreChanged)
  // True when this city groups MASQUE servers rather than WireGuard ones. Used
  // by the server list to show a protocol badge.
  Q_PROPERTY(bool isMasque READ isMasque CONSTANT)

 public:
  ServerCity();
  ServerCity(const ServerCity& other);
  ServerCity& operator=(const ServerCity& other);
  ~ServerCity();

  [[nodiscard]] bool fromJson(const QJsonObject& obj, const QString& country);
  // Build a MASQUE city from a Remote Settings city entry. Its servers are
  // referenced by hostname (MASQUE servers have no public key) and the whole
  // city is tagged with the MASQUE protocol.
  [[nodiscard]] bool fromMasqueJson(const QJsonObject& obj,
                                    const QString& country);

  bool initialized() const { return !m_name.isEmpty(); }

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  const QString& country() const { return m_country; }

  // Human-facing city name. For MASQUE cities the identity name (name()) is the
  // country name to avoid colliding with WireGuard cities, so the real city is
  // kept separately here and used for display only. Falls back to name().
  const QString& displayName() const {
    return m_displayName.isEmpty() ? m_name : m_displayName;
  }

  // Map a MASQUE city code (an IATA/ICAO airport code from Remote Settings) to
  // a real city name. Returns an empty string for unknown/generic codes.
  static QString masqueCityName(const QString& code);

  static QString localizedName(const QString& name);
  const QString localizedName() const { return localizedName(displayName()); }

  const QString& hashKey() const { return m_hashKey; }
  static QString hashKey(const QString& country, const QString cityName);

  double latitude() const { return m_latitude; }

  double longitude() const { return m_longitude; }

  Server::ProtocolType protocol() const { return m_protocol; }

  bool isMasque() const { return m_protocol == Server::ProtocolType::Masque; }

  const QList<QString> servers() const { return m_servers; }

  void setLatency(qint64 msec);
  qint64 latency() const { return m_latency; }

  void setConnectionScore(int score);
  int connectionScore() const { return m_connectionScore; }

 signals:
  void latencyChanged() const;
  void scoreChanged() const;

 private:
  QString m_country;
  QString m_name;
  QString m_code;
  QString m_hashKey;
  QString m_displayName;
  double m_latitude;
  double m_longitude;
  Server::ProtocolType m_protocol = Server::ProtocolType::WireGuard;

  QList<QString> m_servers;

  // Settable field for connection scoring.
  qint64 m_latency = 0;
  int m_connectionScore = 0;
};

#endif  // SERVERCITY_H
