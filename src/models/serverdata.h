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

  Q_PROPERTY(QString exitCountryCode READ exitCountryCode NOTIFY changed)
  Q_PROPERTY(QString exitCityName READ exitCityName NOTIFY changed)
  Q_PROPERTY(QString localizedExitCountryName READ localizedExitCountryName
                 NOTIFY retranslationNeeded)
  Q_PROPERTY(QString localizedExitCityName READ localizedExitCityName NOTIFY
                 retranslationNeeded)

  Q_PROPERTY(bool multihop READ multihop NOTIFY changed)

  Q_PROPERTY(QString entryCountryCode READ entryCountryCode NOTIFY changed)
  Q_PROPERTY(QString entryCityName READ entryCityName NOTIFY changed)
  Q_PROPERTY(QString localizedEntryCountryName READ localizedEntryCountryName
                 NOTIFY retranslationNeeded)
  Q_PROPERTY(QString localizedEntryCityName READ localizedEntryCityName NOTIFY
                 retranslationNeeded)

  Q_PROPERTY(QString previousExitCountryCode READ previousExitCountryCode NOTIFY
                 changed)
  Q_PROPERTY(
      QString previousExitCityName READ previousExitCityName NOTIFY changed)
  Q_PROPERTY(QString localizedPreviousExitCountryName READ
                 localizedPreviousExitCountryName NOTIFY retranslationNeeded)
  Q_PROPERTY(QString localizedPreviousExitCityName READ
                 localizedPreviousExitCityName NOTIFY retranslationNeeded)

 public:
  ServerData();
  ServerData(const ServerData& other);
  ~ServerData();

  ServerData& operator=(const ServerData& other);

  [[nodiscard]] bool fromSettings();

  Q_INVOKABLE void changeServer(const QString& countryCode,
                                const QString& cityName,
                                const QString& entryCountryCode = QString(),
                                const QString& entryCityName = QString());
  bool hasServerData() const { return !m_exitCountryCode.isEmpty(); }

  const QList<Server> exitServers() const;
  const QList<Server> entryServers() const;

  const QString& exitCountryCode() const { return m_exitCountryCode; }
  const QString& exitCityName() const { return m_exitCityName; }
  QString localizedExitCountryName() const;
  QString localizedExitCityName() const;

  bool multihop() const {
    return !m_entryCountryCode.isEmpty() && !m_entryCityName.isEmpty();
  }

  const QString& entryCountryCode() const { return m_entryCountryCode; }
  const QString& entryCityName() const { return m_entryCityName; }
  QString localizedEntryCityName() const;
  QString localizedEntryCountryName() const;

  const QString& previousExitCountryCode() const {
    return m_previousExitCountryCode;
  }
  QString localizedPreviousExitCountryName() const;
  const QString& previousExitCityName() const { return m_previousExitCityName; }
  QString localizedPreviousExitCityName() const;

  void forget();

  void update(const QString& exitCountryCode, const QString& exitCityName,
              const QString& entryCountryCode = QString(),
              const QString& entryCityName = QString());

  void retranslate() { emit retranslationNeeded(); }

  void setEntryServerPublicKey(const QString& publicKey);
  void setExitServerPublicKey(const QString& publicKey);

  const QString& exitServerPublicKey() const { return m_exitServerPublicKey; }
  const QString& entryServerPublicKey() const { return m_entryServerPublicKey; }

 signals:
  void changed();
  void retranslationNeeded();

 private:
  bool settingsChanged();
  static QList<Server> getServerList(const QString& countryCode,
                                     const QString& cityName);

 private:
  QString m_exitCountryCode;
  QString m_exitCountryName;
  QString m_exitCityName;

  QString m_entryCountryCode;
  QString m_entryCountryName;
  QString m_entryCityName;

  QString m_previousExitCountryCode;
  QString m_previousExitCountryName;
  QString m_previousExitCityName;

  QString m_exitServerPublicKey;
  QString m_entryServerPublicKey;
};

#endif  // SERVERDATA_H
