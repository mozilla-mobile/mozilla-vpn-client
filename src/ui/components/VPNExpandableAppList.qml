/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

ColumnLayout{
    id: appListContainer

    property var listModel: undefined
    property var header: ""
    property var description: ""
    property var actionText: ""
    property var onAction: ()=>{}
    property var isEnabled: true
    property var isListVisible : true && applist.count > 0
    opacity: isEnabled ? 1 : 0.5
    property var isActionEnabled: isEnabled && applist.count > 0

    VPNClickableRow {
        id: appRow
        Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
        handleMouseClick: function() { isListVisible = !isListVisible && applist.count > 0; }
        handleKeyClick: function() { isListVisible = !isListVisible && applist.count > 0; }
        clip: true
        canGrowVertical: true
        accessibleName: name
        Layout.preferredWidth: parent.width - Theme.windowMargin
        height: appRowHeader.height
        Layout.preferredHeight: Theme.rowHeight * 1.5


        RowLayout {
            id: appRowHeader
            width: parent.width
            anchors.centerIn: parent
            spacing: 0

            VPNIcon {
                id: toggleArrow
                Layout.leftMargin: defaultMargin / 2
                Layout.rightMargin: defaultMargin - 5
                source: "../resources/arrow-toggle.svg"
                transformOrigin: Image.Center
                smooth: true
                opacity: applist.count > 0 ?1:0;
                rotation: isListVisible ? 0 :-90
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
                        visible: !isListVisible || !isEnabled
                        text: " (%0)".arg(applist.count)
                    }
                }

                VPNTextBlock {
                    id: enabledAppSubtext
                    text: description // "These apps will (not) use vpn.."
                    Layout.fillWidth: true
                }
            }
            VPNLinkButton{
                Layout.alignment: Qt.AlignRight
                // (Un)protect-All Button
                labelText: actionText
                onClicked: {
                    onAction();
                    isListVisible=false
                }
                enabled: isActionEnabled
                opacity: isActionEnabled ? 1 : 0
            }
        }
    }

    VPNList {
        id: applist
        model: listModel
        Layout.fillWidth: true
        spacing: 26
        listName: header

        Layout.preferredHeight: contentItem.childrenRect.height
        Layout.topMargin: defaultMargin

        visible: isListVisible

        delegate: VPNCheckBoxRow {
            labelText: appName
            subLabelText: appID
            isChecked: appIsEnabled
            isEnabled: appListContainer.isEnabled
            showDivider:false
            onClicked: VPNAppPermissions.flip(appID)
            visible: true
            width: parent.width
            anchors.left: parent.left
            anchors.topMargin: defaultMargin
            leftMargin:defaultMargin
            iconURL: "image://app/"+appID
            subLabelWrapMode: Text.NoWrap
        }
    }
}
