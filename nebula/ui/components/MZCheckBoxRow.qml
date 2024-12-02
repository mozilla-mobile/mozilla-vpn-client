/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

RowLayout {
    id: checkBoxRow

    property string labelText
    property string subLabelText
    property bool isChecked
    property bool showDivider: true
    property int leftMargin: 18
    property alias labelWidth: labelWrapper.width

    signal clicked()

    spacing: MZTheme.theme.windowMargin

    MZCheckBox {
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

        MZInterLabel {
            id: label
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            text: labelText
            color: MZTheme.colors.fontColorDark
            horizontalAlignment: Text.AlignLeft
            visible: !!labelText.length

            MZMouseArea {
                anchors.fill: undefined
                width: parent.implicitWidth
                height: parent.implicitHeight
                propagateClickToParent: false
                onClicked: checkBox.clicked()
            }
        }

        MZTextBlock {
            id: subLabel

            font.pixelSize: label.visible ? MZTheme.theme.fontSizeSmall : MZTheme.theme.fontSize
            Layout.fillWidth: true
            text: subLabelText
            visible: !!subLabelText.length
            wrapMode: Text.Wrap
            verticalAlignment: label.visible ? Text.AlignTop : Text.AlignVCenter
            Layout.topMargin: label.visible || (!label.visible && lineCount > 1) ? 0 : MZTheme.theme.checkBoxRowSubLabelTopMargin //accounts for the topMargin of the MZCheckbox

            MZMouseArea {
                enabled: !label.visible
                anchors.fill: undefined
                width: parent.implicitWidth
                height: parent.implicitHeight
                propagateClickToParent: false
                onClicked: checkBox.clicked()
            }
        }

        Rectangle {
            id: divider

            Layout.topMargin: MZTheme.theme.windowMargin
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: MZTheme.colors.divider
            visible: showDivider
        }

    }

}
