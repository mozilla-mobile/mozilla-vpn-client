/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    id: checkBoxRow

    property var labelText
    property var subLabelText: ""
    property bool isChecked
    property bool isEnabled: true
    property bool showDivider: true
    property var leftMargin: 18
    property bool showAppImage: false

    signal clicked()

    spacing: Theme.windowMargin

    Rectangle {
        Layout.preferredHeight:  Theme.vSpacing
        Layout.preferredWidth: Theme.vSpacing
        Layout.leftMargin: showAppImage ? 0 : leftMargin
        Layout.alignment: Qt.AlignTop
        color: "transparent"

        VPNCheckBox {
            id: checkBox

            anchors.centerIn: parent
            onClicked: checkBoxRow.clicked()
            checked: isChecked
            enabled: isEnabled
            opacity: isEnabled ? 1 : 0.5
        }
    }

    Image {
//      source: "image://app/"+appID
        source: "../resources/connection-info.svg"
//        visible: showAppImage && appID !== ""
        visible: showAppImage
        sourceSize.width: Theme.windowMargin * 2
        sourceSize.height: Theme.windowMArgin * 2
        Layout.alignment: Qt.AlignTop
        asynchronous: true
        fillMode:  Image.PreserveAspectFit
    }

    ColumnLayout {
        id: labelWrapper

        Layout.fillWidth: true
        Layout.topMargin: 2
        spacing: 4

        VPNInterLabel {
            id: label
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            text: labelText
            color: Theme.fontColorDark
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 15
        }

        VPNTextBlock {
            id: subLabel

            Layout.fillWidth: true
            text: subLabelText
            visible: !!subLabelText.length
            wrapMode: Text.WordWrap
        }

        Rectangle {
            id: divider

            Layout.topMargin: Theme.windowMargin
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "#E7E7E7"
            visible: showDivider
        }

    }

}
