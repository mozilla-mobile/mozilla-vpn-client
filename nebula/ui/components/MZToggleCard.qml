/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

Item {
    property alias labelText: label.text
    property alias sublabelText: sublabel.text
    property alias toggleEnabled: toggle.enabled
    property alias toggleChecked: toggle.checked
    property alias toggleObjectName: toggle.objectName

    MZDropShadowWithStates {
        anchors.fill: rect
        source: rect
        z: -2
        state: "overwrite-state"
        opacity: .2
    }

    Rectangle {
        id: rect
        anchors.fill: enableAppList
        anchors.topMargin: -MZTheme.theme.windowMargin
        anchors.bottomMargin: anchors.topMargin
        anchors.leftMargin: -MZTheme.theme.windowMargin
        anchors.rightMargin: anchors.leftMargin
        color: MZTheme.theme.white
        radius: 4
    }

    RowLayout {
        id: enableAppList
        anchors.top: parent.top
        anchors.topMargin: MZTheme.theme.windowMargin + (rect.anchors.topMargin *-1)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: MZTheme.theme.windowMargin * 2
        anchors.rightMargin: MZTheme.theme.windowMargin * 2
        spacing: MZTheme.theme.windowMargin

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            MZInterLabel {
                id: label
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                color: MZTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
            }

            MZTextBlock {
                id: sublabel
                Layout.fillWidth: true
                width: undefined
            }
        }

        MZSettingsToggle {
            id: toggle

            Layout.preferredHeight: 24
            Layout.preferredWidth: 45
            checked: (!MZSettings.protectSelectedApps)
            onClicked: handleClick()
            accessibleName: label.text
        }
    }
}
