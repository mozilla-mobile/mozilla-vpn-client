/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import Qt5Compat.GraphicalEffects

ColorOverlay {
  // See https://forum.qt.io/topic/127404/qt-6-replacement-for-coloroverlay
  // Fixes "Warning: Texture 0x7f21e601ac70 () used with different accesses within the same pass, this is not allowed."
  cached: true
}
