/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "netmgrdevice.h"

#include "leakdetector.h"
#include "logger.h"
#include "netmgrtypes.h"

namespace {
Logger logger("NetmgrDevice");
}

#define DBUS_NM_DEVICE (DBUS_NM_SERVICE ".Device")

NetmgrDevice::NetmgrDevice(const QString& path, QObject* parent)
    : QObject(parent), m_interface(DBUS_NM_SERVICE, path, DBUS_NM_DEVICE,
                                   QDBusConnection::systemBus()) {
  MZ_COUNT_CTOR(NetmgrDevice);

  // Report state changes.
  QDBusConnection::systemBus().connect(
      m_interface.service(), path, m_interface.interface(), "StateChanged",
      this, SLOT(dbusStateChanged(uint, uint, uint)));

  // Watch for property changes
  m_interface.connection().connect(
      DBUS_NM_SERVICE, path, "org.freedesktop.DBus.Properties",
      "PropertiesChanged", this,
      SLOT(propertyChanged(QString, QVariantMap, QStringList)));

  m_state = m_interface.property("State").toUInt();

  QVariant qv = m_interface.property("ActiveConnection");
  updateActiveConnection(qv.value<QDBusObjectPath>());
}

NetmgrDevice::~NetmgrDevice() {
  MZ_COUNT_DTOR(NetmgrDevice);
}

void NetmgrDevice::updateActiveConnection(const QDBusObjectPath& path) {
  m_activeConnection = path.path();
  QDBusInterface conn(DBUS_NM_SERVICE, m_activeConnection,
                      QStringLiteral(DBUS_NM_SERVICE) + ".Connection.Active",
                      m_interface.connection());

  QVariant value = conn.property("Uuid");
  if (value.metaType() == QMetaType::fromType<QString>()) {
    m_uuid = value.toString();
  } else {
    m_uuid = QString();
  }
}

void NetmgrDevice::propertyChanged(QString interface, QVariantMap changes,
                                   QStringList list) {
  Q_UNUSED(list);
  if (interface != DBUS_NM_DEVICE) {
    return;
  }
  if (changes.contains("ActiveConnection")) {
    QVariant qv = changes.value("ActiveConnection");
    activeConnectionChanged(qv.value<QDBusObjectPath>());
  }
}

void NetmgrDevice::dbusStateChanged(uint state, uint prev, uint reason) {
  m_state = state;
  emit stateChanged(state, prev, reason);
}
