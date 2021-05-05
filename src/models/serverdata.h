/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERDATA_H
#define SERVERDATA_H

#include <QObject>

class ServerCountryModel;
class ServerCountry;
class ServerCity;
class Server;

class ServerData final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerData);

  Q_PROPERTY(QString countryCode READ countryCode NOTIFY changed)
  Q_PROPERTY(QString cityName READ cityName NOTIFY changed)
  Q_PROPERTY(QString localizedCityName READ localizedCityName NOTIFY changed)

 public:
  ServerData();
  ~ServerData();

  [[nodiscard]] bool fromSettings();

  void writeSettings();

  void initialize(const ServerCountry& country, const ServerCity& city);

  bool initialized() const { return m_initialized; }

  const QString& countryCode() const { return m_countryCode; }

  const QString& countryName() const { return m_countryName; }

  const QString& cityName() const { return m_cityName; }

  QString localizedCityName() const;

  void forget() { m_initialized = false; }

  void update(const QString& countryCode, const QString& countryName,
              const QString& cityName);

 signals:
  void changed();

 private:
  void initializeInternal(const QString& countryCode,
                          const QString& countryName, const QString& cityName);

 private:
  bool m_initialized = false;

  QString m_countryCode;
  QString m_countryName;
  QString m_cityName;
};

#endif  // SERVERDATA_H
