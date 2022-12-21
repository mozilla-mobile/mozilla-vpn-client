/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonproperty.h"

#include "leakdetector.h"

AddonProperty::AddonProperty() { MZ_COUNT_CTOR(AddonProperty); }

AddonProperty::~AddonProperty() { MZ_COUNT_DTOR(AddonProperty); }

void AddonProperty::initialize(const QString& id, const QString& fallback) {
  m_initialized = true;
  m_id = id;
  m_fallback = fallback;
}

QString AddonProperty::get() const {
  Q_ASSERT(m_initialized);

  QString str = qtTrId(m_id.toUtf8());
  if (!str.isEmpty() && str != m_id) {
    return str;
  }

  return m_fallback;
}

void AddonProperty::set(const QString& id, const QString& fallback) {
  m_id = id;
  m_fallback = fallback;
}
