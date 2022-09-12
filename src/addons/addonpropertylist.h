/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONPROPERTYLIST_H
#define ADDONPROPERTYLIST_H

#include <QList>
#include <QObject>
#include <QPair>

#define ADDON_PROPERTY_LIST(name, member, getter, setter, inserter, appender, \
                            remover, signal)                                  \
  Q_PROPERTY(QStringList name READ getter NOTIFY signal)                      \
  QStringList getter() { return member.get(); }                               \
  Q_INVOKABLE void setter(int pos, const QString& id,                         \
                          const QString& fallback) {                          \
    return member.set(pos, id, fallback);                                     \
    emit signal();                                                            \
  }                                                                           \
  Q_INVOKABLE void inserter(int pos, const QString& id,                       \
                            const QString& fallback) {                        \
    return member.insert(pos, id, fallback);                                  \
    emit signal();                                                            \
  }                                                                           \
  Q_INVOKABLE void appender(const QString& id, const QString& fallback) {     \
    return member.append(id, fallback);                                       \
    emit signal();                                                            \
  }                                                                           \
  Q_INVOKABLE void remover(int pos) {                                         \
    return member.remove(pos);                                                \
    emit signal();                                                            \
  }

class AddonPropertyList final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonPropertyList)

  Q_PROPERTY(QStringList value READ get CONSTANT)

 public:
  AddonPropertyList();
  ~AddonPropertyList();

  void set(int pos, const QString& id, const QString& fallback);
  void insert(int pos, const QString& id, const QString& fallback);
  void append(const QString& id, const QString& fallback);
  void remove(int pos);

  QStringList get() const;

 private:
  QList<QPair<QString, QString>> m_list;
};

#endif  // ADDONPROPERTYLIST_H
