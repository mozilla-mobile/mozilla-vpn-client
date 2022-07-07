/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcherlocales.h"
#include "leakdetector.h"
#include "localizer.h"
#include "settingsholder.h"

// static
AddonConditionWatcher* AddonConditionWatcherLocales::maybeCreate(
    QObject* parent, const QStringList& locales) {
  if (locales.isEmpty()) {
    return nullptr;
  }

  return new AddonConditionWatcherLocales(parent, locales);
}

AddonConditionWatcherLocales::AddonConditionWatcherLocales(
    QObject* parent, const QStringList& locales)
    : AddonConditionWatcher(parent), m_locales(locales) {
  MVPN_COUNT_CTOR(AddonConditionWatcherLocales);

  connect(SettingsHolder::instance(), &SettingsHolder::languageCodeChanged,
          this, [this]() {
            bool newStatus = conditionApplied();
            if (m_currentStatus != newStatus) {
              m_currentStatus = newStatus;
              emit conditionChanged(m_currentStatus);
            }
          });
}

AddonConditionWatcherLocales::~AddonConditionWatcherLocales() {
  MVPN_COUNT_DTOR(AddonConditionWatcherLocales);
}

bool AddonConditionWatcherLocales::conditionApplied() const {
  QString code = SettingsHolder::instance()->languageCode();
  if (code.isEmpty()) {
    code = QLocale::system().bcp47Name();
    if (code.isEmpty()) {
      code = "en";
    }
  }

  code = Localizer::majorLanguageCode(code);
  return m_locales.contains(code.toLower());
}
