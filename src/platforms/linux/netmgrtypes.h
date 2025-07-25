/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRTYPES_H
#define NETMGRTYPES_H

#include <QHostAddress>
#include <QMap>
#include <QMetaType>
#include <QVariant>
#include <QtDBus>

#define DBUS_NM_SERVICE "org.freedesktop.NetworkManager"
#define DBUS_NM_PATH "/org/freedesktop/NetworkManager"
#define DBUS_NM_INTERFACE "org.freedesktop.NetworkManager"

// Metatype registration glue
template<typename T>
class NetmgrType {
 public:
  operator QVariant() const { return toVariant(); }
  QVariant toVariant() const {
    const T* value = reinterpret_cast<const T*>(this);
    return QVariant::fromValue<T>(*value);
  };

 protected:
  explicit NetmgrType() { Q_UNUSED(s_registration); }

  class NetmgrTypeRegistration {
   public:
    NetmgrTypeRegistration() {
      Q_UNUSED(s_registration);
      qRegisterMetaType<T>();
      qDBusRegisterMetaType<T>();
    };
  };
  static inline NetmgrTypeRegistration s_registration;
};

class NetmgrConfig : public QMap<QString,QVariantMap>, public NetmgrType<NetmgrConfig> {
 public:
  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const NetmgrConfig& data) {
    args.beginMap(QMetaType::QString, QMetaType::QVariantMap);
    for (auto i = data.constBegin(); i != data.constEnd(); i++) {
      args.beginMapEntry();
      args << i.key() << i.value();
      args.endMapEntry();
    }
    args.endMap();
    return args;
  }

  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         NetmgrConfig& data) {
    data.clear();
    args.beginMap();
    while (!args.atEnd()) {
      args.beginMapEntry();
      QString key;
      QVariantMap value;
      args >> key >> value;
      data.insert(key, value);
      args.endMapEntry();
    }
    args.endMap();
    return args;
  }
};
Q_DECLARE_METATYPE(NetmgrConfig);

class NetmgrDataList : public QList<QVariantMap>, public NetmgrType<NetmgrDataList> {
 public:
  NetmgrDataList() : QList<QVariantMap>() {};
  NetmgrDataList(const QVariantMap& data) : QList<QVariantMap>({data}) {};

  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const NetmgrDataList& data) {
    args.beginArray(QMetaType::QVariantMap);
    for (const auto& entry : data) {
      args << entry;
    }
    args.endArray();
    return args;
  }

  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         NetmgrDataList& data) {
    data.clear();
    args.beginArray();
    while (!args.atEnd()) {
      QVariantMap value;
      args >> value;
      data.append(value);
    }
    args.endArray();
    return args;
  }
};
Q_DECLARE_METATYPE(NetmgrDataList);

class NetmgrIpv6List : public QList<Q_IPV6ADDR>, public NetmgrType<NetmgrIpv6List> {
 public:
  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const NetmgrIpv6List& data) {
    args.beginArray(QMetaType::QByteArray);
    for (const auto& entry : data) {
      args << QByteArray((const char*)&entry, sizeof(Q_IPV6ADDR));
    }
    args.endArray();
    return args;
  }

  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         NetmgrIpv6List& data) {
    data.clear();
    args.beginArray();
    while (!args.atEnd()) {
      QByteArray value;
      args >> value;
      if ((size_t)value.length() >= sizeof(Q_IPV6ADDR)) {
        Q_IPV6ADDR addr;
        memcpy(&addr, value.constData(), sizeof(addr));
        data.append(addr);
      }
    }
    args.endArray();
    return args;
  }
};
Q_DECLARE_METATYPE(NetmgrIpv6List);

#endif  // NETMGRTYPES_H
