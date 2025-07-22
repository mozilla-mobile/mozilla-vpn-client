/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NETMGRTYPES_H
#define NETMGRTYPES_H

#include <QVariant>
#include <QMap>
#include <QMetaType>

// Metatype registration glue
template<typename T>
class NetMgrType {
 public:
  QVariant toVariant() const {
    const T* value = reinterpret_cast<const T*>(this);
    return QVariant::fromValue<T>(*value);
  };

 protected:
  explicit NetMgrType() { Q_UNUSED(s_registration); }

  class NetMgrTypeRegistration {
   public:
    NetMgrTypeRegistration() {
      Q_UNUSED(s_registration);
      qRegisterMetaType<T>();
      qDBusRegisterMetaType<T>();
    };
  };
  static inline NetMgrTypeRegistration s_registration;
};

class NetMgrConfig : public QMap<QString,QVariantMap>, public NetMgrType<NetMgrConfig> {
 public:
  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const NetMgrConfig& data) {
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
                                         NetMgrConfig& data) {
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
Q_DECLARE_METATYPE(NetMgrConfig);

class NetMgrDataList : public QList<QVariantMap>, public NetMgrType<NetMgrDataList> {
 public:
  NetMgrDataList() : QList<QVariantMap>() {};
  NetMgrDataList(const QVariantMap& data) : QList<QVariantMap>({data}) {};

  friend QDBusArgument& operator<<(QDBusArgument& args,
                                   const NetMgrDataList& data) {
    args.beginArray(QMetaType::QVariantMap);
    for (const auto& entry : data) {
      args << entry;
    }
    args.endArray();
    return args;
  }

  friend const QDBusArgument& operator>>(const QDBusArgument& args,
                                         NetMgrDataList& data) {
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
Q_DECLARE_METATYPE(NetMgrDataList);

#endif  // NETMGRTYPES_H
