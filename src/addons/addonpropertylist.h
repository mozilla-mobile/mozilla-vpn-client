/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONPROPERTYLIST_H
#define ADDONPROPERTYLIST_H

#include <QList>
#include <QObject>
#include <QPair>

#define ADDON_PROPERTY_LIST(name, member, signal)                        \
  Q_PROPERTY(                                                            \
      QStringList name READ addonPropertyListGetter##name NOTIFY signal) \
  QStringList addonPropertyListGetter##name() { return member.get(); }

class AddonPropertyList final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonPropertyList)

  Q_PROPERTY(QStringList value READ get CONSTANT)

 public:
  AddonPropertyList();
  ~AddonPropertyList();

  void append(const QString& id, const QString& fallback);

  QStringList get() const;

 private:
  QList<QPair<QString, QString>> m_list;
};

#endif  // ADDONPROPERTYLIST_H
