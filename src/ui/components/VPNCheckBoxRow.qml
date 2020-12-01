/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

RowLayout {
    id: checkBoxRow

    property var labelText
    property var subLabelText
    property bool isChecked
    property bool isEnabled: true
    property bool showDivider: true

    signal clicked()
    spacing: 0

    VPNCheckBox {
        id: checkBox

        Layout.leftMargin: 18
        onClicked: checkBoxRow.clicked()
        checked: isChecked
        enabled: isEnabled
        opacity: isEnabled ? 1 : 0.5
    }

    ColumnLayout {
        id: labelWrapper

        Layout.fillWidth: true
        spacing: 0

        VPNInterLabel {
            id: label
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            text: labelText
            color: Theme.fontColorDark
            horizontalAlignment: Text.AlignLeft
        }

        VPNTextBlock {
            id: subLabel

            Layout.fillWidth: true
            text: subLabelText
            visible: !!subLabelText.length
        }

        Rectangle {
            id: divider

            Layout.topMargin: 16
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "#E7E7E7"
            visible: showDivider
        }

    }

}
