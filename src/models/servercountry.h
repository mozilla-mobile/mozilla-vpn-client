/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCOUNTRY_H
#define SERVERCOUNTRY_H

#include "servercity.h"

#include <QHash>
#include <QList>
#include <QString>

class ServerData;
class ServerExtra;
class QJsonObject;

class ServerCountry final {
 public:
  ServerCountry();
  ServerCountry(const ServerCountry& other);
  ServerCountry& operator=(const ServerCountry& other);
  ~ServerCountry();

  [[nodiscard]] bool fromJson(const QJsonObject& obj,
                              const QHash<QString, ServerExtra>& serverExtras);

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  const QList<ServerCity>& cities() const { return m_cities; }

  const QList<Server> servers(const ServerData& data) const;

  void sortCities();

 private:
  QString m_name;
  QString m_code;

  QList<ServerCity> m_cities;
};

#endif  // SERVERCOUNTRY_H
