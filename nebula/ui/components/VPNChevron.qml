/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0

// VPNChevron
Image {
    source: "qrc:/nebula/resources/chevron.svg"
    mirror: VPNLocalizer.isRightToLeft
    sourceSize.height: 24
    sourceSize.width: 24
    fillMode: Image.PreserveAspectFit
}
