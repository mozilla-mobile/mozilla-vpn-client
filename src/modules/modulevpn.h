/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULEVPN_H
#define MODULEVPN_H

#include "module.h"

class ModuleVPN final : public Module {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ModuleVPN)

 public:
  explicit ModuleVPN(QObject* parent);
  ~ModuleVPN();
};

#endif  // MODULEVPN_H
