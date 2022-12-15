/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

VPNIconButton {
    accessibleName: VPNl18n.GlobalPaste

    Image {
        anchors.centerIn: parent
        fillMode: Image.PreserveAspectFit
        source: "qrc:/nebula/resources/paste.svg"
        sourceSize.height: VPNTheme.theme.iconSize * 1.5
        sourceSize.width: VPNTheme.theme.iconSize * 1.5
    }
}
