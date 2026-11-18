/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PINGSENDERFACTORY_H
#define PINGSENDERFACTORY_H

#include "pingsender.h"

class PingSenderFactory final {
 public:
  PingSenderFactory() = delete;
  static PingSender* create(PingSender::NetworkProtocol proto, QObject* parent);
};

#endif  // PINGSENDERFACTORY_H
