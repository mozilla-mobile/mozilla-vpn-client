/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DBUSTYPES_H
#define DBUSTYPES_H

#include <sys/socket.h>

#include <QByteArray>
#include <QDBusArgument>
#include <QHostAddress>
#include <QtDBus/QtDBus>

class SystemdUnitProp {
 public:
  SystemdUnitProp(){};
  SystemdUnitProp(const QString& n, const QVariant& v) : name(n), value(v){};
  QString name;
  QVariant value;

  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const SystemdUnitProp& data) {
    args.beginStructure();
    args << data.name << QDBusVariant(data.value);
    args.endStructure();
    return args;
  }
  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         SystemdUnitProp& data) {
    QDBusVariant dv;
    args.beginStructure();
    args >> data.name >> dv;
    args.endStructure();
    data.value = dv.variant();
    return args;
  }
};
typedef QList<SystemdUnitProp> SystemdUnitPropList;
Q_DECLARE_METATYPE(SystemdUnitProp);
Q_DECLARE_METATYPE(SystemdUnitPropList);

class SystemdUnitAux {
 public:
  QString name;
  SystemdUnitPropList prop;

  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const SystemdUnitAux& data) {
    args.beginStructure();
    args << data.name << data.prop;
    args.endStructure();
    return args;
  }
  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         SystemdUnitAux& data) {
    args.beginStructure();
    args >> data.name >> data.prop;
    args.endStructure();
    return args;
  }
};
typedef QList<SystemdUnitAux> SystemdUnitAuxList;
Q_DECLARE_METATYPE(SystemdUnitAux);
Q_DECLARE_METATYPE(SystemdUnitAuxList);

class DBusMetatypeRegistrationProxy {
 public:
  DBusMetatypeRegistrationProxy() {
    qRegisterMetaType<SystemdUnitProp>();
    qDBusRegisterMetaType<SystemdUnitProp>();
    qRegisterMetaType<SystemdUnitPropList>();
    qDBusRegisterMetaType<SystemdUnitPropList>();
    qRegisterMetaType<SystemdUnitAux>();
    qDBusRegisterMetaType<SystemdUnitAux>();
    qRegisterMetaType<SystemdUnitAuxList>();
    qDBusRegisterMetaType<SystemdUnitAuxList>();
  }
};

#endif  // DBUSTYPESLINUX_H
