/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "collator.h"

#ifdef MVPN_IOS
#  include "platforms/ios/iosutils.h"
#endif

#if defined(MVPN_WASM)
#  include "settingsholder.h"
#  include <emscripten.h>

EM_JS(int, vpnWasmCompareString,
      (const char* a, const char* b, const char* languageCode), {
        return UTF8ToString(a).localeCompare(UTF8ToString(b),
                                             UTF8ToString(languageCode),
                                             {sentitivity : "base"});
      });
#endif

int Collator::compare(const QString& a, const QString& b) {
  // On iOS, the standard QT package for arm does not link ICU. Let's have our
  // own collator implementation based on NSStrings.
#if defined(MVPN_IOS)
  return IOSUtils::compareStrings(a, b);
#elif defined(MVPN_WASM)
  // For WASM, we have a similar issue (no ICU). Let's use the JS API to sort
  // strings.
  QString languageCode = SettingsHolder::instance().languageCode();
  if (languageCode.isEmpty()) {
    languageCode = QLocale::system().bcp47Name();
  }

  return vpnWasmCompareString(a.toLocal8Bit().constData(),
                              b.toLocal8Bit().constData(),
                              languageCode.toLocal8Bit().constData());
#else
  return m_collator.compare(a, b);
#endif
}
