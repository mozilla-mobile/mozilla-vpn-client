/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LANGUAGEI18N_H
#define LANGUAGEI18N_H

#include <QString>

class LanguageI18N final {
 public:
  static bool languageExists(const QString& languageCode);

  static QString translateLanguage(const QString& translationCode,
                                   const QString& languageCode);

  static int languageCompare(const QString& languageCodeA,
                             const QString& languageCodeB);
};

#endif  // LANGUAGEI18N_H
