/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settingsconnector.h"

#include "leakdetector.h"

SettingsConnector::SettingsConnector(QObject* parent, QSettings* settings)
    : QObject(parent), m_settings(settings) {
  MZ_COUNT_CTOR(SettingsConnector);
}

SettingsConnector::~SettingsConnector() { MZ_COUNT_DTOR(SettingsConnector); }

QVariant SettingsConnector::getValue(const QString& key) const {
  return m_settings->value(key);
}

void SettingsConnector::setValue(const QString& key, QVariant value) const {
  m_settings->setValue(key, value);
}

void SettingsConnector::remove(const QString& key, const QString& group) const {
  m_settings->beginGroup(group);
  m_settings->remove(key);
  m_settings->endGroup();
}

QStringList SettingsConnector::getAllKeys(const QString& group) const {
  m_settings->beginGroup(group);
  auto keys = m_settings->allKeys();
  m_settings->endGroup();

  return keys;
}

bool SettingsConnector::contains(const QString& key) const {
  return m_settings->contains(key);
}
