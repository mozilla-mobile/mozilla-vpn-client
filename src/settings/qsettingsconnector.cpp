/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "qsettingsconnector.h"

#include "leakdetector.h"

QSettingsConnector::QSettingsConnector(QObject* parent, QSettings* settings)
    : QObject(parent), m_settings(settings) {
  MZ_COUNT_CTOR(QSettingsConnector);
}

QSettingsConnector::~QSettingsConnector() { MZ_COUNT_DTOR(QSettingsConnector); }

QVariant QSettingsConnector::getValue(const QString& key) const {
  return m_settings->value(key);
}

void QSettingsConnector::setValue(const QString& key, QVariant value) const {
  m_settings->setValue(key, value);
}

void QSettingsConnector::remove(const QString& key,
                                const QString& group) const {
  m_settings->beginGroup(group);
  m_settings->remove(key);
  m_settings->endGroup();
}

QStringList QSettingsConnector::getAllKeys(const QString& group) const {
  m_settings->beginGroup(group);
  auto keys = m_settings->allKeys();
  m_settings->endGroup();

  return keys;
}

bool QSettingsConnector::contains(const QString& key) const {
  return m_settings->contains(key);
}
