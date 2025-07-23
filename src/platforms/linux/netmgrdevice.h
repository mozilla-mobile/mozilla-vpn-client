/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRDEVICE_H
#define NETMGRDEVICE_H

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QObject>

class NetMgrDevice final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NetMgrDevice)

 public:
  NetMgrDevice(const QString& path, QObject* parent = nullptr);
  ~NetMgrDevice();

  enum State {
    UNKNOWN = 0,
    UNMANAGED = 10,
    UNAVAILABLE = 20,
    DISCONNECTED = 30,
    PREPARE = 40,
    CONFIG = 50,
    NEED_AUTH = 60,
    IP_CONFIG = 70,
    IP_CHECK = 80,
    SECONDARIES = 90,
    ACTIVATED = 100,
    DEACTIVATING = 110,
    FAILED = 120,
  };
  Q_ENUM(State);

  QString name() const { return m_interface.property("Interface").toString(); }
  QString driver() const { return m_interface.property("Driver").toString(); }
  State state() const {
    return static_cast<State>(m_interface.property("State").toUInt());
  }

  QString path() const { return m_interface.path(); }
  QString uuid() const { return m_uuid; }

 signals:
  void stateChanged(uint state, uint prev, uint reason);

 private slots:
  void dbusStateChanged(uint state, uint prev, uint reason);
  void activeConnectionChanged(const QDBusObjectPath& path);
  void propertyChanged(QString interface, QVariantMap props, QStringList list);

 private:
  QDBusInterface m_interface;
  QString m_uuid;
};

#endif  // NETMGRDEVICE_H
