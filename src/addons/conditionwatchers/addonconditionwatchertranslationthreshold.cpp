/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertranslationthreshold.h"

#include <QDir>
#include <QFileInfo>

#include "addonconditionwatcherlocales.h"
#include "addons/addon.h"
#include "translations/localizer.h"
#include "utilities/leakdetector.h"

// static
AddonConditionWatcher* AddonConditionWatcherTranslationThreshold::maybeCreate(
    Addon* addon, double translationThreshold) {
  if (translationThreshold == 0) {
    return nullptr;
  }

  QStringList locales;
  const QMap<QString, double>& map = addon->translationCompleteness();
  QMapIterator<QString, double> i(map);
  while (i.hasNext()) {
    i.next();

    if (i.value() >= translationThreshold) {
      locales.append(i.key());
      continue;
    }

    QStringList languageCodeFallback =
        Localizer::instance()->fallbackForLanguage(i.key());

    for (const QString& languageCode : languageCodeFallback) {
      if (map.value(languageCode, 0) >= translationThreshold) {
        locales.append(i.key());
        break;
      }
    }
  }

  return AddonConditionWatcherLocales::maybeCreate(
      addon, locales, AddonConditionWatcherLocales::CheckMajorLanguageCode);
}
