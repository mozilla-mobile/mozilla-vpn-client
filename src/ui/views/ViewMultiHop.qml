/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import "../themes/colors.js" as Color
import "../components"

Item {
    VPNBoldLabel {
        id: txt
        text: "MULTI - HOP"
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -56
        font.pixelSize: 36
        font.letterSpacing: 2
    }
}
