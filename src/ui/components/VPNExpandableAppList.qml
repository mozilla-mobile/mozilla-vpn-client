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
    property var actionText: ""
    property var onAction: ()=>{}
    property var isEnabled: true

    state: listVisible ? "list-visible" : "list-hidden"


    Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
    handleMouseClick: function() { listVisible = !listVisible; }
    handleKeyClick: function() { listVisible = !listVisible; }
    clip: true
    canGrowVertical: true
    accessibleName: name
    states: [
        State {
            name: "list-hidden"
            PropertyChanges {
                target: toggleArrow
                rotation: -90

            }
            PropertyChanges {
                target: appRow
                height: appRowHeader.height
            }

        },
        State {
            name: "list-visible"

            PropertyChanges {
                target: toggleArrow
                rotation: 0

            }
            PropertyChanges {
                target: appRow
                height: appRowHeader.height +applist.contentHeight + applist.anchors.topMargin
            }

        }
    ]


    RowLayout {
        id: appRowHeader
        anchors.top: parent.top
        anchors.left: parent.left
        width: appRow.width
        spacing: 0
        VPNServerListToggle {
            id: toggleArrow
            Layout.leftMargin: 15 - enabledAppRow.anchors.leftMargin
            Layout.rightMargin: 15
        }
        ColumnLayout {
            Layout.alignment: Qt.AlignLeft
            Layout.fillWidth: true
            spacing: 0

            RowLayout {
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                VPNInterLabel {
                    id: label
                    text: header
                    Accessible.role: Accessible.Heading
                    color: Theme.fontColorDark
                    horizontalAlignment: Text.AlignLeft
                }

                VPNTextBlock{
                    visible: !listVisible || applist.count == 0
                    text: " (%0)".arg(applist.count)
                }
            }

            VPNTextBlock {
                id: enabledAppSubtext
                text: description
                Layout.fillWidth: true
            }
        }
        VPNLinkButton{
            Layout.alignment: Qt.AlignRight
            // (Un)protect-All Button
            labelText: actionText
            onClicked: {onAction()}
            enabled: isEnabled
            opacity: isEnabled ? 1: 0.5
        }
    }

    VPNList {
        id: applist
        model: listModel

        anchors.top: appRowHeader.bottom
        anchors.topMargin: count > 0 ? 16 : 0
        anchors.left: appRowHeader.left
        width: appRow
        height: contentItem.childrenRect.height
        spacing: 26
        listName: header

        visible: listVisible && count > 0

        delegate: VPNCheckBoxRow {
            labelText: appName
            subLabelText: appID
            isChecked: appIsEnabled
            isEnabled: appRow.isEnabled
            showDivider:false
            onClicked: VPNAppPermissions.flip(appID)
            visible: true
            width: applist.width
            anchors.left: applist.left
            anchors.topMargin: defaultMargin
            leftMargin:12.5
            iconURL: "image://app/"+appID
        }
    }
}
