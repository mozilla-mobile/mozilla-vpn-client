/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "i18nstrings.h"

#include <QCoreApplication>

namespace {
I18nStrings* s_instance = nullptr;
}

// static
I18nStrings* I18nStrings::instance() {
  if (!s_instance) {
    s_instance = new I18nStrings(qApp);
  }

  return s_instance;
}

// static
void I18nStrings::initialize() {
#ifndef BUILD_QMAKE
  Q_INIT_RESOURCE(languages);
  Q_INIT_RESOURCE(servers);
  Q_INIT_RESOURCE(translations);
#endif
}

I18nStrings::I18nStrings(QObject* parent) : QQmlPropertyMap(parent) {
  retranslate();
}

const char* I18nStrings::id(I18nStrings::String string) const {
  Q_ASSERT(string < __Last);
  return _ids[string];
}

QString I18nStrings::t(I18nStrings::String string) const {
  Q_ASSERT(string < __Last);
  QString id = _ids[string];
  if (id.isEmpty()) return "";
  return qtTrId(_ids[string]);
}
