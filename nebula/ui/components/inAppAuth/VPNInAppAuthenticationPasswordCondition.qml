/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

RowLayout {
    id: root
    property alias _iconVisible: icon.visible
    property bool _passwordConditionIsSatisfied: false
    property alias _passwordConditionDescription: passwordConditionDescription.text

    Rectangle {
        Layout.preferredHeight: VPNTheme.theme.iconSize * 1.25
        Layout.preferredWidth: VPNTheme.theme.iconSize * 1.25
        Layout.alignment: Qt.AlignTop
        VPNIcon {
            id: icon
            source: root._passwordConditionIsSatisfied ? "qrc:/nebula/resources/check-green70.svg" : "qrc:/nebula/resources/x-red50.svg"
            anchors.fill: parent
        }
    }

    Text {
        id: passwordConditionDescription

        color: VPNTheme.theme.fontColor
        font.family: VPNTheme.theme.fontInterFamily
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        Layout.fillWidth: true
        lineHeight: 1
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    }
}
