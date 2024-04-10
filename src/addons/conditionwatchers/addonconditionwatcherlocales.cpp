/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcherlocales.h"

#include "localizer.h"
#include "settingsholder.h"
#include "utils/leakdetector/leakdetector.h"

// static
AddonConditionWatcher* AddonConditionWatcherLocales::maybeCreate(
    QObject* parent, const QStringList& locales,
    MajorLanguageCodePolicy majorLanguageCodePolicy) {
  if (locales.isEmpty()) {
    return nullptr;
  }

  return new AddonConditionWatcherLocales(parent, locales,
                                          majorLanguageCodePolicy);
}

AddonConditionWatcherLocales::AddonConditionWatcherLocales(
    QObject* parent, const QStringList& locales,
    MajorLanguageCodePolicy majorLanguageCodePolicy)
    : AddonConditionWatcher(parent),
      m_locales(locales),
      m_majorLanguageCodePolicy(majorLanguageCodePolicy) {
  MZ_COUNT_CTOR(AddonConditionWatcherLocales);

  m_currentStatus = conditionApplied();

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
  MZ_COUNT_DTOR(AddonConditionWatcherLocales);
}

bool AddonConditionWatcherLocales::conditionApplied() const {
  QString code = Localizer::instance()->languageCodeOrSystem();
  Q_ASSERT(!code.isEmpty());

  if (m_locales.contains(code)) {
    return true;
  }

  if (m_majorLanguageCodePolicy == CheckMajorLanguageCode) {
    code = Localizer::majorLanguageCode(code);
    return m_locales.contains(code.toLower());
  }

  return false;
}
