/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRCONNECTION_H
#define NETMGRCONNECTION_H

#include <QDBusInterface>
#include <QObject>

class NetMgrConnection final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetMgrConnection)

 public:
  NetMgrConnection(const QString& path, QObject* parent = nullptr);
  ~NetMgrConnection();

  const QString& uuid() const { return m_uuid; }
  QString path() const { return m_interface.path(); }
  uint state() const;

  enum ActiveState {
    NM_ACTIVE_CONNECTION_STATE_UNKNOWN = 0,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATING = 1,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATED = 2,
    NM_ACTIVE_CONNECTION_STATE_DEACTIVATING = 3,
    NM_ACTIVE_CONNECTION_STATE_DEACTIVATED = 4,
  };
  Q_ENUM(ActiveState);

 signals:
  void stateChanged(uint state, uint reason);

 private slots:
  void dbusStateChanged(uint state, uint reason);

 private:
  QDBusInterface m_interface;
  QString m_uuid;
};

#endif  // NETMGRCONNECTION_H
