/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

RowLayout {
    id: root
    property alias _iconVisible: icon.visible
    property bool _passwordConditionIsSatisfied: false
    property alias _passwordConditionDescription: passwordConditionDescription.text

    Rectangle {
        Layout.preferredHeight: MZTheme.theme.iconSize * 1.25
        Layout.preferredWidth: MZTheme.theme.iconSize * 1.25
        Layout.alignment: Qt.AlignTop
        MZIcon {
            id: icon
            source: root._passwordConditionIsSatisfied ? "qrc:/nebula/resources/check-green70.svg" : "qrc:/nebula/resources/x-red50.svg"
            anchors.fill: parent
        }
    }

    Text {
        id: passwordConditionDescription

        color: MZTheme.colors.fontColor
        font.family: MZTheme.theme.fontInterFamily
        font.pixelSize: MZTheme.theme.fontSizeSmall
        Layout.fillWidth: true
        lineHeight: 1
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        Accessible.ignored: !visible
    }
}
