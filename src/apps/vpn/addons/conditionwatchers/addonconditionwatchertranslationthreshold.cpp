/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatchertranslationthreshold.h"

#include <QDir>
#include <QFileInfo>

#include "addon.h"
#include "addonconditionwatcherlocales.h"
#include "leakdetector.h"
#include "localizer.h"

// static
AddonConditionWatcher* AddonConditionWatcherTranslationThreshold::maybeCreate(
    Addon* addon, double translationThreshold) {
  if (translationThreshold == 0) {
    return nullptr;
  }

  QFileInfo manifestFileInfo(addon->manifestFileName());
  QDir addonPath = manifestFileInfo.dir();
  if (!addonPath.cd("i18n")) {
    return nullptr;
  }

  QStringList locales;
  QMap<QString, double> map = Localizer::loadTranslationCompleteness(
      addonPath.filePath("translations.completeness"));
  QMapIterator<QString, double> i(map);
  while (i.hasNext()) {
    i.next();
    if (i.value() >= translationThreshold) {
      locales.append(i.key());
    }
  }

  return AddonConditionWatcherLocales::maybeCreate(addon, locales);
}
