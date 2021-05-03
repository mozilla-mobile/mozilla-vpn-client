/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRYMODEL_H
#define SERVERCOUNTRYMODEL_H

#include "servercountry.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QObject>
#include <QPointer>

class ServerData;

class ServerCountryModel final : public QAbstractListModel {
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

  void pickRandom(ServerData& data) const;

  bool pickIfExists(const QString& countryCode, const QString& cityCode,
                    ServerData& data) const;

  // For windows data migration.
  bool pickByIPv4Address(const QString& ipv4Address, ServerData& data) const;

  bool exists(ServerData& data) const;

  const QList<Server> servers(const ServerData& data) const;

  const QString countryName(const QString& countryCode) const;

  const QList<ServerCountry>& countries() const { return m_countries; }

  void retranslate();

  // For the web-extension
  const QByteArray& rawJson() const { return m_rawJson; }

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override {
    return m_countries.length();
  }

  QVariant data(const QModelIndex& index, int role) const override;

 private:
  [[nodiscard]] bool fromJsonInternal(const QByteArray& data);

  void sortCountries();

 private:
  QByteArray m_rawJson;

  QList<ServerCountry> m_countries;
};

#endif  // SERVERCOUNTRYMODEL_H
