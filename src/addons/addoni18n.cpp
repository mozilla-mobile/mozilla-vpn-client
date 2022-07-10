/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addoni18n.h"
#include "leakdetector.h"
#include "localizer.h"

AddonI18n::AddonI18n(QObject* parent, const QString& manifestFileName,
                     const QString& id, const QString& name)
    : Addon(parent, manifestFileName, id, name, "i18n") {
  MVPN_COUNT_CTOR(AddonI18n);
}

AddonI18n::~AddonI18n() { MVPN_COUNT_DTOR(AddonI18n); }

void AddonI18n::enable() {
  emit Localizer::instance()->codeChanged();
  Addon::enable();
}

void AddonI18n::disable() {
  emit Localizer::instance()->codeChanged();
  Addon::disable();
}
