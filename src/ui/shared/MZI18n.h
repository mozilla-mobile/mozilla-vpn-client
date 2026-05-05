/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <QQmlEngine>

#include "i18nstrings.h"

struct MZI18n {
  Q_GADGET
  QML_FOREIGN(I18nStrings)
  QML_NAMED_ELEMENT(MZI18n)
  QML_SINGLETON

 public:
  static I18nStrings* create(QQmlEngine*, QJSEngine*) {
    return I18nStrings::instance();
  }
};
