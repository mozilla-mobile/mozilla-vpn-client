/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "l18nstrings.h"

#include <QCoreApplication>

namespace {
L18nStrings* s_instance = nullptr;
}

// static
L18nStrings* L18nStrings::instance() {
  if (!s_instance) {
    s_instance = new L18nStrings(qApp);
  }

  return s_instance;
}

// static
void L18nStrings::initialize() {
#ifndef BUILD_QMAKE
  Q_INIT_RESOURCE(servers);
  Q_INIT_RESOURCE(translations);
#endif
}

L18nStrings::L18nStrings(QObject* parent) : QQmlPropertyMap(parent) {
  retranslate();
}

const char* L18nStrings::id(L18nStrings::String string) const {
  Q_ASSERT(string < __Last);
  return _ids[string];
}

QString L18nStrings::t(L18nStrings::String string) const {
  Q_ASSERT(string < __Last);
  QString id = _ids[string];
  if (id.isEmpty()) return "";
  return qtTrId(_ids[string]);
}
