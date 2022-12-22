/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonpropertylist.h"

#include "leakdetector.h"

AddonPropertyList::AddonPropertyList() { MZ_COUNT_CTOR(AddonPropertyList); }

AddonPropertyList::~AddonPropertyList() { MZ_COUNT_DTOR(AddonPropertyList); }

void AddonPropertyList::set(int pos, const QString& id,
                            const QString& fallback) {
  if (pos >= 0 && pos < m_list.length()) {
    QPair<QString, QString> value(id, fallback);
    m_list[pos].swap(value);
  }
}

void AddonPropertyList::insert(int pos, const QString& id,
                               const QString& fallback) {
  if (pos >= 0 && pos <= m_list.length()) {
    QPair<QString, QString> value(id, fallback);
    m_list.insert(pos, value);
  }
}

void AddonPropertyList::append(const QString& id, const QString& fallback) {
  m_list.append(QPair<QString, QString>(id, fallback));
}

void AddonPropertyList::remove(int pos) {
  if (pos >= 0 && pos < m_list.length()) {
    m_list.removeAt(pos);
  }
}

QStringList AddonPropertyList::get() const {
  QStringList list;

  for (const QPair<QString, QString>& pair : m_list) {
    QString str = qtTrId(pair.first.toUtf8());
    if (!str.isEmpty() && str != pair.first) {
      list.append(str);
      continue;
    }

    list.append(pair.second);
  }

  return list;
}
