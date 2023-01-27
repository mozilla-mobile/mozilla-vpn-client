/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

// VPNExternalLinkListItem
VPNClickableRow {
    property alias title: title.text
    property string iconSource:  "qrc:/nebula/resources/externalLink.svg"
    property alias iconMirror: icon.mirror

    backgroundColor: VPNTheme.theme.clickableRowBlue

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        anchors.rightMargin: VPNTheme.theme.windowMargin / 2

        VPNBoldLabel {
            id: title
        }

        Item {
            Layout.fillWidth: true
        }

        VPNIcon {
            id: icon
            source: iconSource
        }

    }

}
