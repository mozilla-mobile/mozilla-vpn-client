/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRY_H
#define SERVERCOUNTRY_H

#include <QList>
#include <QString>

#include "servercity.h"

class QJsonObject;

class ServerCountry final {
 public:
  ServerCountry();
  ServerCountry(const ServerCountry& other);
  ServerCountry& operator=(const ServerCountry& other);
  ~ServerCountry();

  [[nodiscard]] bool fromJson(const QJsonObject& obj);

  // Set up an empty country (no cities yet). Used when merging MASQUE locations
  // into a country that is not already present in the model.
  void initialize(const QString& code, const QString& name);

  // Add a city name to this country if it is not already listed, keeping the
  // city list sorted. Used to merge MASQUE cities into an existing country.
  void addCity(const QString& cityName);

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  static QString localizedName(const QString& code, const QString& name);
  QString localizedName() const { return localizedName(m_code, m_name); }

  const QList<QString>& cities() const { return m_cities; }

  void sortCities();

 private:
  QString m_name;
  QString m_code;

  QList<QString> m_cities;
};

#endif  // SERVERCOUNTRY_H
