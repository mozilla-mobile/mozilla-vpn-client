/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

RowLayout {

    property bool _passwordConditionIsSatisfied: false
    property alias _passwordConditionDescription: passwordConditionDescription.text

    VPNIcon {
        source: parent._passwordConditionIsSatisfied ? "qrc:/nebula/resources/check-green70.svg" : "qrc:/nebula/resources/x-red50.svg"
        sourceSize.width: VPNTheme.theme.iconSize * 1.25
        sourceSize.height: VPNTheme.theme.iconSize * 1.25
        Layout.alignment: Qt.AlignTop
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
