/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QObject>

#include "servercountry.h"

class Location;

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

  Q_INVOKABLE QStringList pickRandom() const;
  QStringList pickBest(const Location& location) const;

  bool exists(const QString& countryCode, const QString& cityName) const;

  const QList<Server> servers(const QString& countryCode,
                              const QString& cityName) const;
  const QList<Server> servers() const { return m_servers.values(); };
  Server server(const QString& pubkey) const { return m_servers.value(pubkey); }

  const QString countryName(const QString& countryCode) const;

  Q_INVOKABLE QString getLocalizedCountryName(const QString& countryCode);

  const QString localizedCountryName(const QString& countryCode) const;

  const QList<ServerCountry>& countries() const { return m_countries; }

  void retranslate();
  void setServerLatency(const QString& publicKey, unsigned int msec);
  void setServerCooldown(const QString& publicKey);
  void setCooldownForAllServersInACity(const QString& countryCode,
                                       const QString& cityCode);

  Q_INVOKABLE int cityConnectionScore(const QString& countryCode,
                                      const QString& cityCode) const;

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override {
    return static_cast<int>(m_countries.length());
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
