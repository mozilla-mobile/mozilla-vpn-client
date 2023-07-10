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

  ServerCountryModel();
  ~ServerCountryModel();

  [[nodiscard]] bool fromSettings();

  [[nodiscard]] bool fromJson(const QByteArray& data);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  QStringList pickBest() const;

  bool exists(const QString& countryCode, const QString& cityName) const;
  const ServerCity& findCity(const QString& countryCode,
                             const QString& cityName) const;

  const Server& server(const QString& pubkey) const;

  const QString countryName(const QString& countryCode) const;

  const QHash<QString, ServerCity>& cities() const { return m_cities; }

  const QList<ServerCountry>& countries() const { return m_countries; }

  void retranslate();
  void setCooldownForAllServersInACity(const QString& countryCode,
                                       const QString& cityCode);

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

 private:
  QByteArray m_rawJson;

  QList<ServerCountry> m_countries;
  QHash<QString, ServerCity> m_cities;
  QHash<QString, Server> m_servers;
};

#endif  // SERVERCOUNTRYMODEL_H
