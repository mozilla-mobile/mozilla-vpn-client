/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include <QAbstractListModel>
#include <QByteArray>
#include <QObject>

#include "models/servercountry.h"

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

  [[nodiscard]] bool fromJson(const QByteArray& data);
  bool appendFromJson(const QByteArray& json);

  // Merge MASQUE servers, fetched from the Remote Settings "vpn-serverlist"
  // changeset, into the model. MASQUE locations appear as extra cities under
  // their country, tagged with the MASQUE protocol. The raw changeset is kept
  // so it can be re-applied whenever the (WireGuard) server list is reloaded.
  bool appendMasqueServers(const QByteArray& changeset);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  bool exists(const QString& countryCode, const QString& cityName) const;
  ServerCity& findCity(const QString& countryCode, const QString& cityName);
  const ServerCity& findCity(const QString& countryCode,
                             const QString& cityName) const;

  const Server& server(const QString& pubkey) const;

  const QString countryName(const QString& countryCode) const;

  const QHash<QString, ServerCity>& cities() const { return m_cities; }

  const QList<ServerCountry>& countries() const { return m_countries; }

  void retranslate();

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override {
    return static_cast<int>(m_countries.length());
  }

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

 private:
  [[nodiscard]] bool fromJsonInternal(const QByteArray& data, bool append);

  // Merge the parsed MASQUE changeset into the current lists without touching
  // model-reset signalling; callers wrap this in begin/endResetModel.
  bool mergeMasqueChangeset(const QByteArray& changeset);

  void sortCountries();

 private:
  QByteArray m_rawJson;
  // Raw MASQUE changeset, re-applied after every (WireGuard) list reload.
  QByteArray m_masqueRawJson;

  QList<ServerCountry> m_countries;
  QHash<QString, ServerCity> m_cities;
  QHash<QString, Server> m_servers;
};

#endif  // SERVERCOUNTRYMODEL_H
