/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonpropertylist.h"
#include "leakdetector.h"

AddonPropertyList::AddonPropertyList() { MVPN_COUNT_CTOR(AddonPropertyList); }

AddonPropertyList::~AddonPropertyList() { MVPN_COUNT_DTOR(AddonPropertyList); }

void AddonPropertyList::append(const QString& id, const QString& fallback) {
  m_list.append(QPair<QString, QString>(id, fallback));
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
