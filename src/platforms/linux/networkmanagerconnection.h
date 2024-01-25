/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETWORKMANAGERCONNECTION_H
#define NETWORKMANAGERCONNECTION_H

#include <QDBusInterface>
#include <QObject>

class NetworkManagerConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetworkManagerConnection)

 public:
  NetworkManagerConnection(const QString& path, QObject* parent = nullptr);
  ~NetworkManagerConnection();

  const QString& uuid() const { return m_uuid; }
  QString path() const { return m_interface.path(); }
  uint state() const;

 signals:
  void stateChanged(uint state, uint reason);

 private slots:
  void dbusStateChanged(uint state, uint reason);

 private:
  QDBusInterface m_interface;
  QString m_uuid;
};

#endif  // NETWORKMANAGERCONNECTION_H
