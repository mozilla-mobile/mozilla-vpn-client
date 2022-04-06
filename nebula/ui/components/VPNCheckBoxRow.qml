/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    id: checkBoxRow

    property bool labelVisible: true
    property var labelText: ""
    property var subLabelText: ""
    property bool isChecked
    property bool isEnabled: true
    property bool showDivider: true
    property var leftMargin: 18
    property bool showAppImage: false
    property alias labelWidth: labelWrapper.width

    signal clicked()

    spacing: VPNTheme.theme.windowMargin

    VPNCheckBox {
        id: checkBox
        onClicked: checkBoxRow.clicked()
        checked: isChecked
        enabled: isEnabled
        opacity: isEnabled ? 1 : 0.5
        Component.onCompleted: {
            if (!showAppImage) {
                Layout.leftMargin = leftMargin
            }
        }
    }

    Rectangle {
        visible: showAppImage
        Layout.preferredWidth: VPNTheme.theme.windowMargin * 2
        Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
        color: VPNTheme.theme.transparent
        radius: 4
        Layout.alignment: Qt.AlignTop

        Image {
            sourceSize.width: VPNTheme.theme.windowMargin * 2
            sourceSize.height: VPNTheme.theme.windowMargin * 2
            anchors.centerIn: parent
            asynchronous: true
            fillMode:  Image.PreserveAspectFit
            Component.onCompleted: {
                if (showAppImage && appID !== "") {
                    source = "image://app/"+appID
                }
            }
        }
    }

    ColumnLayout {
        id: labelWrapper

        Layout.fillWidth: true
        Layout.topMargin: 2
        spacing: 4
        Layout.alignment: Qt.AlignTop

        VPNInterLabel {
            id: label
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            text: labelText
            visible: labelVisible
            color: VPNTheme.theme.fontColorDark
            horizontalAlignment: Text.AlignLeft

            VPNMouseArea {
                propagateClickToParent: false
                onClicked: checkBox.clicked()
            }
        }

        VPNTextBlock {
            id: subLabel

            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            text: subLabelText
            visible: !!subLabelText.length
            wrapMode: showAppImage ? Text.WrapAtWordBoundaryOrAnywhere : Text.WordWrap
        }

        Rectangle {
            id: divider

            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "#E7E7E7"
            visible: showDivider
        }

    }

}
