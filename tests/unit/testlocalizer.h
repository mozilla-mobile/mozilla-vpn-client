/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "helper.h"

class SettingsHolder;

class TestLocalizer final : public TestHelper {
  Q_OBJECT

 private slots:
  void init();
  void cleanup();

  void basic();

  void systemLanguage();

  void localizeCurrency();

  void majorLanguageCode();

  void parseBCP47Languages_data();
  void parseBCP47Languages();

  void parseIOSLanguages_data();
  void parseIOSLanguages();

  void completeness_data();
  void completeness();

  void fallback();

  void formattedDate_data();
  void formattedDate();

  void nativeLanguageName_data();
  void nativeLanguageName();

  void localizedLanguageName();

  void getTranslatedCountryName();

  void getTranslatedCityName();

 private:
  SettingsHolder* m_settingsHolder = nullptr;
};
