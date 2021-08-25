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

  Q_PROPERTY(QString exitCountryCode READ exitCountryCode NOTIFY changed)
  Q_PROPERTY(QString exitCityName READ exitCityName NOTIFY changed)
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

  const QString& exitCountryCode() const { return m_exitCountryCode; }

  const QString& exitCityName() const { return m_exitCityName; }

  QString localizedCityName() const;

  bool multihop() const {
    return !m_entryCountryCode.isEmpty() && !m_entryCityName.isEmpty();
  }

  const QString& entryCountryCode() const { return m_entryCountryCode; }

  const QString& entryCityName() const { return m_entryCityName; }

  QString localizedEntryCity() const;

  void forget() { m_initialized = false; }

  void update(const QString& exitCountryCode, const QString& exitCityName,
              const QString& entryCountryCode = QString(),
              const QString& entryCityName = QString());

  QString toString() const;

 signals:
  void changed();

 private:
  void initializeInternal(const QString& exitCountryCode,
                          const QString& exitCityName,
                          const QString& entryCountryCode,
                          const QString& entryCityName);

 private:
  bool m_initialized = false;

  QString m_exitCountryCode;
  QString m_exitCityName;

  QString m_entryCountryCode;
  QString m_entryCityName;
};

#endif  // SERVERDATA_H
