/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "settings/setting.h"

#include "leakdetector.h"
#include "settingsbase.h"

Setting::Setting(QObject* parent, const QString& key,
                 std::function<QVariant()> defaultValue, bool removeWhenReset,
                 bool sensitiveSetting)
    : QObject(parent),
      m_key(key),
      m_defaultValue(defaultValue),
      m_sensitiveSetting(sensitiveSetting),
      m_removeWhenReset(removeWhenReset) {
  MZ_COUNT_CTOR(Setting);
}

Setting::~Setting() { MZ_COUNT_DTOR(Setting); }

QVariant Setting::get() const {
  auto value = SettingsBase::instance()->m_settings.value(m_key);

  if (value.isNull()) {
    value = m_defaultValue();
  }

  return value;
}

void Setting::set(QVariant value) const {
  if (!isSet() || get() != value) {
    SettingsBase::instance()->m_settings.setValue(m_key, value);
    emit changed();
  }
}

void Setting::reset() const {
  if (!m_removeWhenReset) {
    return;
  }

  remove();
}

void Setting::remove() const {
  if (!isSet()) {
    return;
  }

  SettingsBase::instance()->m_settings.remove(m_key);
  emit changed();
}

bool Setting::isSet() const {
  return SettingsBase::instance()->m_settings.contains(m_key);
}

QString Setting::log() const {
  if (!isSet()) {
    return "";
  }

  QString logLine;
  if (m_sensitiveSetting) {
    logLine.append(QString("%1 -> <Sensitive>").arg(m_key));
  } else {
    logLine.append(QString("%1 -> ").arg(m_key));
    QVariant value = get();
    switch (value.typeId()) {
      case QVariant::List:
      case QVariant::StringList:
        logLine.append(QString("[%1]").arg(value.toStringList().join(",")));
        break;
      default:
        logLine.append(value.toString());
    }
  }

  return logLine;
}
