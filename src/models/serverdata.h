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

  Q_PROPERTY(bool multihop READ multihop NOTIFY changed)
  Q_PROPERTY(QString entryCountryCode READ entryCountryCode NOTIFY changed)
  Q_PROPERTY(QString entryCityName READ entryCityName NOTIFY changed)
  Q_PROPERTY(QString localizedEntryCity READ localizedEntryCity NOTIFY changed)

 public:
  ServerData();
  ~ServerData();

  [[nodiscard]] bool fromSettings();
  [[nodiscard]] bool fromString(const QString& data);

  void writeSettings();

  bool initialized() const { return m_initialized; }

  const QString& countryCode() const { return m_countryCode; }

  const QString& cityName() const { return m_cityName; }

  QString localizedCityName() const;

  bool multihop() const {
    return !m_entryCountryCode.isEmpty() && !m_entryCityName.isEmpty();
  }

  const QString& entryCountryCode() const { return m_entryCountryCode; }

  const QString& entryCityName() const { return m_entryCityName; }

  QString localizedEntryCity() const;

  void forget() {
    m_initialized = false;
  }

  void update(const QString& countryCode, const QString& cityName,
              const QString& entryCountryCode = QString(),
              const QString& entryCityName = QString());

  QString toString() const;

 signals:
  void changed();

 private:
  void initializeInternal(const QString& countryCode, const QString& cityName,
                          const QString& entryCountryCode,
                          const QString& entryCityName);

 private:
  bool m_initialized = false;

  QString m_countryCode;
  QString m_cityName;

  QString m_entryCountryCode;
  QString m_entryCityName;
};

#endif  // SERVERDATA_H
