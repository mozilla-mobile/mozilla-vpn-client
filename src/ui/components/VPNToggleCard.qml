/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

import org.mozilla.Glean 0.15
import telemetry 0.15

Item {
    property alias labelText: label.text
    property alias sublabelText: sublabel.text
    property alias toggleEnabled: toggle.enabled
    property alias toggleChecked: toggle.checked
    property alias toolTipTitleText: toggle.toolTipTitle
    property alias toggleObjectName: toggle.objectName

    Rectangle {
        id: rect
        anchors.fill: enableAppList
        anchors.topMargin: -Theme.windowMargin
        anchors.bottomMargin: anchors.topMargin
        anchors.leftMargin: -Theme.windowMargin
        anchors.rightMargin: anchors.leftMargin
        color: Theme.white
        radius: 4
    }

    RowLayout {
        id: enableAppList
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin + (rect.anchors.topMargin *-1)
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Theme.windowMargin * 2
        anchors.rightMargin: Theme.windowMargin * 2
        spacing: Theme.windowMargin

        ColumnLayout {
            Layout.alignment: Qt.AlignVCenter
            VPNInterLabel {
                id: label
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                color: Theme.fontColorDark
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
        }
    }
}
