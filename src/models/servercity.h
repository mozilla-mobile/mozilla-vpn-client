/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SERVERCITY_H
#define SERVERCITY_H

#include "server.h"

#include <QList>
#include <QString>

class QJsonObject;

class ServerCity final {
 public:
  ServerCity();
  ServerCity(const ServerCity& other);
  ServerCity& operator=(const ServerCity& other);
  ~ServerCity();

  [[nodiscard]] bool fromJson(const QJsonObject& obj);

  const QString& name() const { return m_name; }

  const QString& code() const { return m_code; }

  double latitude() const { return m_latitude; }

  double longitude() const { return m_longitude; }

  const QList<QString> servers() const { return m_servers; }

 private:
  QString m_name;
  QString m_code;
  double m_latitude;
  double m_longitude;

  QList<QString> m_servers;
};

#endif  // SERVERCITY_H
