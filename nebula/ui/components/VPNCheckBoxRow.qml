/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

RowLayout {
    id: checkBoxRow

    property string labelText
    property string subLabelText
    property bool isChecked
    property bool showDivider: true
    property int leftMargin: 18
    property alias labelWidth: labelWrapper.width

    signal clicked()

    spacing: VPNTheme.theme.windowMargin

    VPNCheckBox {
        id: checkBox

        objectName: "checkbox"
        onClicked: checkBoxRow.clicked()
        checked: isChecked
        opacity: checkBoxRow.enabled ? 1 : 0.5
        accessibleName: labelText
        Layout.leftMargin: leftMargin
    }

    ColumnLayout {
        id: labelWrapper

        Layout.fillWidth: true
        spacing: 4
        Layout.alignment: Qt.AlignTop

        VPNInterLabel {
            id: label
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            text: labelText
            color: VPNTheme.theme.fontColorDark
            horizontalAlignment: Text.AlignLeft
            visible: !!labelText.length

            VPNMouseArea {
                anchors.fill: undefined
                width: parent.implicitWidth
                height: parent.implicitHeight
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
            wrapMode: Text.WordWrap
        }

        Rectangle {
            id: divider

            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: VPNTheme.colors.grey10
            visible: showDivider
        }

    }

}
