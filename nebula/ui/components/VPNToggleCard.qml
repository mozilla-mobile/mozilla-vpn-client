/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

Item {
    property alias labelText: label.text
    property alias sublabelText: sublabel.text
    property alias toggleEnabled: toggle.enabled
    property alias toggleChecked: toggle.checked
    property alias toolTipTitleText: toggle.toolTipTitle
    property alias toggleObjectName: toggle.objectName

    VPNDropShadowWithStates {
        anchors.fill: rect
        source: rect
        z: -2
        state: "overwrite-state"
        opacity: .2
    }

    Rectangle {
        id: rect
        anchors.fill: enableAppList
        anchors.topMargin: -VPNTheme.theme.windowMargin
        anchors.bottomMargin: anchors.topMargin
        anchors.leftMargin: -VPNTheme.theme.windowMargin
        anchors.rightMargin: anchors.leftMargin
        color: VPNTheme.theme.white
        radius: 4
    }

    RowLayout {
        id: enableAppList
        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.windowMargin + (rect.anchors.topMargin *-1)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: VPNTheme.theme.windowMargin * 2
        anchors.rightMargin: VPNTheme.theme.windowMargin * 2
        spacing: VPNTheme.theme.windowMargin

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            VPNInterLabel {
                id: label
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                color: VPNTheme.theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
            }

            VPNTextBlock {
                id: sublabel
                Layout.fillWidth: true
                width: undefined
            }
        }

        VPNSettingsToggle {
            id: toggle

            Layout.preferredHeight: 24
            Layout.preferredWidth: 45
            checked: (!VPNSettings.protectSelectedApps)
            onClicked: handleClick()
            Accessible.name: label.text
            Accessible.description: subLabel.text
            toolTipTitle: label.text
        }
    }
}
