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

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  QString localizedName() const;

  const QList<QString>& cities() const { return m_cities; }

  void sortCities();

 private:
  QString m_name;
  QString m_code;

  QList<QString> m_cities;
};

#endif  // SERVERCOUNTRY_H
