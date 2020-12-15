/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0

VPNClickableRow {
    id: appRow

    property var listVisible : true
    property var listModel: undefined
    property var header: ""
    property var description: ""

    state: listVisible ? "list-visible" : "list-hidden"

    Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
    handleMouseClick: function() { listVisible = !listVisible; }
    handleKeyClick: function() { listVisible = !listVisible; }
    clip: true


    Behavior on y {
        PropertyAnimation {
            duration: 200
        }
    }

    accessibleName: name

    states: [
        State {
            name: "list-hidden"

            PropertyChanges {
                target: appRow
                height: appRow.height
            }

            PropertyChanges {
                target: applist
                opacity: 0
            }
            PropertyChanges {
                target: toggleArrow
                rotation: -90

            }

        },
        State {
            name: "list-visible"

            PropertyChanges {
                target: appRow
                height: appRow.height +applist.height
            }

            PropertyChanges {
                target: applist
                opacity: 1
            }
            PropertyChanges {
                target: toggleArrow
                rotation: 0

            }

        }
    ]


    RowLayout {
        id: appRowHeader
        anchors.top: parent.top
        anchors.left: parent.left
        spacing: 0
        VPNServerListToggle {
            id: toggleArrow
            Layout.leftMargin: 15 - enabledAppRow.anchors.leftMargin
            Layout.rightMargin: 15
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            VPNInterLabel {
                id: label
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true
                text: header
                Accessible.role: Accessible.Heading
                color: Theme.fontColorDark
                horizontalAlignment: Text.AlignLeft
            }
            VPNTextBlock {
                id: enabledAppSubtext
                text: description
                Layout.fillWidth: true
            }
        }
    }


    VPNList {
        id: applist
        model: listModel

        anchors.top: appRowHeader.bottom
        anchors.topMargin: 16
        anchors.left: vpnFlickable.left
        width: parent.width
        height: contentItem.childrenRect.height + 40
        spacing: 26
        listName: header

        visible: count > 0


        delegate: VPNCheckBoxRow {
            labelText: appName
            subLabelText: appID
            isChecked: appIsEnabled
            isEnabled: vpnFlickable.vpnIsOff
            showDivider:false
            onClicked: VPNAppPermissions.flip(appID)
            visible: true
            width: applist.width
            anchors.left: applist.left
            anchors.topMargin: defaultMargin
            leftMargin: 18
            iconURL: "image://app/"+appID
        }
    }
}
