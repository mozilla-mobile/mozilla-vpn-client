/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MZADDONMESSAGE_H
#define MZADDONMESSAGE_H

#include <QQmlEngine>

#include "addons/addonmessage.h"

class AddonMessage;

struct MZAddonMessage {
  Q_GADGET
  QML_FOREIGN(AddonMessage)
  QML_ELEMENT
  QML_UNCREATABLE("")
};

#endif  // MZADDONMESSAGE_H
