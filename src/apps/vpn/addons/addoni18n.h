/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADDONI18N_H
#define ADDONI18N_H

#include "addon.h"

class AddonI18n final : public Addon {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AddonI18n)

 public:
  AddonI18n(QObject* parent, const QString& manifestFileName, const QString& id,
            const QString& name);

  ~AddonI18n();

 private:
  void enable() override;
  void disable() override;
};

#endif  // ADDONI18N_H
