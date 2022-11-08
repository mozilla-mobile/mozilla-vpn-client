/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include "servercountry.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QObject>

class ServerData;

class ServerCountryModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ServerCountryModel)

 public:
  enum ServerCountryRoles {
    NameRole = Qt::UserRole + 1,
    LocalizedNameRole,
    CodeRole,
    CitiesRole,
  };

  enum ServerConnectionScores {
    Unavailable = -1,
    NoData = 0,
    Poor = 1,
    Moderate = 2,
    Good = 3,
  };
  Q_ENUM(ServerConnectionScores);

  ServerCountryModel();
  ~ServerCountryModel();

  [[nodiscard]] bool fromSettings();

  [[nodiscard]] bool fromJson(const QByteArray& data);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  Q_INVOKABLE QStringList pickRandom();

  void pickRandom(ServerData& data) const;

  bool pickIfExists(const QString& countryCode, const QString& cityCode,
                    ServerData& data) const;

  // For windows data migration.
  bool pickByIPv4Address(const QString& ipv4Address, ServerData& data) const;

  bool exists(ServerData& data) const;

  const QList<Server> servers(const ServerData& data) const;
  const QList<Server> servers() const { return m_servers.values(); };
  Server server(const QString& pubkey) const { return m_servers.value(pubkey); }

  const QString countryName(const QString& countryCode) const;

  Q_INVOKABLE QString getLocalizedCountryName(const QString& countryCode);

  const QString localizedCountryName(const QString& countryCode) const;

  const QList<ServerCountry>& countries() const { return m_countries; }

  void retranslate();
  void setServerLatency(const QString& publicKey, unsigned int msec);
  void setServerCooldown(const QString& publicKey, unsigned int duration);
  void setCooldownForAllServersInACity(const QString& countryCode,
                                       const QString& cityCode,
                                       unsigned int duration);

  Q_INVOKABLE int cityConnectionScore(const QString& countryCode,
                                      const QString& cityCode) const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override {
    return m_countries.length();
  }

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

 private:
  [[nodiscard]] bool fromJsonInternal(const QByteArray& data);

  void sortCountries();
  int cityConnectionScore(const ServerCity& city) const;

 private:
  QByteArray m_rawJson;

  QList<ServerCountry> m_countries;
  QHash<QString, Server> m_servers;
};

#endif  // SERVERCOUNTRYMODEL_H
