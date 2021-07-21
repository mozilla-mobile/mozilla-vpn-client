/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "l18nstrings.h"
#include "leakdetector.h"

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

L18nStrings::L18nStrings(QObject* parent) : QObject(parent) {
  MVPN_COUNT_CTOR(L18nStrings);
}

L18nStrings::~L18nStrings() { MVPN_COUNT_DTOR(L18nStrings); }

QString L18nStrings::tr(L18nStrings::String string) const {
  Q_ASSERT(string < __Last);
  return qtTrId(_ids[string]);
}
