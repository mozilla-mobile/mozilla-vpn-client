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
  Q_PROPERTY(int connectionScore READ connectionScore NOTIFY scoreChanged)

 public:
  ServerCity();
  ServerCity(const ServerCity& other);
  ServerCity& operator=(const ServerCity& other);
  ~ServerCity();

  enum CityConnectionScores {
    Unavailable = -1,
    NoData = 0,
    Poor = 1,
    Moderate = 2,
    Good = 3,
    Excellent = 4,
  };
  Q_ENUM(CityConnectionScores);

  [[nodiscard]] bool fromJson(const QJsonObject& obj, const QString& country);

  bool initialized() const { return !m_name.isEmpty(); }

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  const QString& country() const { return m_country; }

  const QString localizedName() const;

  const QString& hashKey() const { return m_hashKey; }
  static QString hashKey(const QString& country, const QString cityName);

  double latitude() const { return m_latitude; }

  double longitude() const { return m_longitude; }

  int connectionScore() const;

  Q_INVOKABLE int multiHopScore(const QString& country,
                                const QString& cityName) const;

  unsigned int latency() const;

  const QList<QString> servers() const { return m_servers; }

 signals:
  void scoreChanged() const;

 private:
  int baseCityScore(const QString& originCountryCode = QString()) const;

  QString m_country;
  QString m_name;
  QString m_code;
  QString m_hashKey;
  double m_latitude;
  double m_longitude;

  QList<QString> m_servers;
};

#endif  // SERVERCITY_H
