/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

RowLayout {
    id: toggleRow

    property alias labelText: label.text
    property alias subLabelText: subLabel.text
    property alias checked: toggle.checked
    property alias showDivider: divider.visible
    property alias linkButtonText: linkButtonBlock.labelText
    property alias toggleEnabled: toggle.enabled
    property int dividerTopMargin

    signal clicked()
    signal clickedWhileDisabled()
    signal buttonClicked()

    spacing: 16

    ColumnLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop

        spacing: 0

        MZInterLabel {
            id: label

            Layout.fillWidth: true

            text: labelText
            color: MZTheme.colors.fontColorDark
            horizontalAlignment: Text.AlignLeft
            visible: !!text.length
        }

        MZTextBlock {
            id: subLabel

            Layout.fillWidth: true

            font.pixelSize: MZTheme.theme.fontSizeSmall
            visible: !!text.length
            wrapMode: Text.Wrap
            verticalAlignment: Text.AlignVCenter
        }

        MZLinkButton {
            id: linkButtonBlock

            fontSize: MZTheme.theme.fontSizeSmall
            visible: !!labelText.length
            textAlignment: Text.AlignLeft
            onClicked: toggleRow.buttonClicked()
            leftPadding: 0
            topPadding: 3
        }

        Rectangle {
            id: divider

            Layout.topMargin: dividerTopMargin
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            color: MZTheme.colors.divider
        }
    }
    Item {
        Layout.alignment: Qt.AlignTop
        Layout.preferredWidth: toggle.implicitWidth
        Layout.preferredHeight: toggle.implicitHeight

        MZSettingsToggle {
            id: toggle
            objectName: "toggle"
            anchors.fill: parent
            accessibleName: labelText
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
              if (toggle.enabled == false) {
                toggleRow.clickedWhileDisabled()
              } else {
                toggleRow.clicked()
              }
            }
        }
    }
}
