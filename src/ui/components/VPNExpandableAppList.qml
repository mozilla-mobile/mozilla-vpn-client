/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.14
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

ColumnLayout {
    id: appListContainer

    property var listModel: undefined
    property var header: ""
    property var description: ""
    property var actionText: ""
    property var onAction: ()=>{}
    property var isEnabled: true
    property var isListVisible : true && applist.count > 0
    property var isActionEnabled: isEnabled && applist.count > 0
    property var dividerVisible: false

    opacity: isEnabled && applist.count > 0 ? 1 : 0.5
    anchors.horizontalCenter: parent.horizontalCenter
    width: parent.width
    spacing: 0

    Behavior on y {
        PropertyAnimation {
            duration: 200
        }
    }

    VPNClickableRow {
        id: appRow
        Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
        handleMouseClick: function() { isListVisible = !isListVisible && applist.count > 0; }
        handleKeyClick: function() { isListVisible = !isListVisible && applist.count > 0; }
        canGrowVertical: true
        accessibleName: description
        Layout.preferredWidth: parent.width - Theme.windowMargin
        Layout.alignment: Qt.AlignHCenter
        Layout.minimumHeight: Theme.rowHeight * 1.5
        enabled: vpnIsOff && applist.count > 0
        anchors.left: undefined
        anchors.right: undefined
        anchors.rightMargin: undefined
        anchors.leftMargin: undefined

        ColumnLayout {
            id: appRowHeader
            width: appRow.width
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0
            RowLayout {
                spacing: 0
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                Behavior on y {
                    PropertyAnimation {
                        duration: 100
                    }
                }
                VPNIcon {
                    id: toggleArrow
                    Layout.leftMargin: defaultMargin / 3
                    Layout.rightMargin: defaultMargin - 4
                    Layout.alignment: Qt.AlignVCenter

                    source: "../resources/arrow-toggle.svg"
                    transformOrigin: Image.Center
                    smooth: true
                    opacity: applist.count > 0 ?1:0;
                    rotation: isListVisible ? 0 :-90
                }

                RowLayout {
                    Layout.alignment: Qt.AlignLeft
                    spacing: 0

                    VPNBoldLabel {
                        id: label
                        text: header
                        Accessible.role: Accessible.Heading
                        color: Theme.fontColorDark
                        horizontalAlignment: Text.AlignLeft
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                    }

                    VPNTextBlock{
                        text: " (%0)".arg(applist.count)
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true
                    }
                }
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: 2
                visible: isListVisible
            }

            VPNTextBlock {
                id: enabledAppSubtext
                text: description // "These apps will (not) use vpn.."
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width
                width: undefined
                visible: isListVisible
                Layout.leftMargin: 44
                wrapMode: Text.Wrap
            }

        }
    }

    VPNLinkButton{
        Layout.alignment: Qt.AlignRight
        fontSize: Theme.fontSizeSmall

        // (Un)protect-All Button
        labelText: actionText
        onClicked: {
            onAction();
            isListVisible=false
        }
        enabled: isActionEnabled
        opacity: isActionEnabled ? 1 : 0
        visible: isListVisible
    }

    VPNList {
        id: applist
        model: listModel
        Layout.fillWidth: true
        spacing: 26
        listName: header

        Layout.preferredHeight: contentItem.childrenRect.height
        Layout.topMargin: 4

        visible: isListVisible && count  > 0

        PropertyAnimation on opacity {
            duration: 200
        }

        removeDisplaced: Transition {
              NumberAnimation {
                  properties: "x,y"
                  duration: 200
              }

          }
          remove: Transition {
              PropertyAnimation {
                  property: "opacity"
                  from: 1
                  to: 0
                  duration: 200
              }
          }
        addDisplaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 200
            }
        }
        add: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        delegate: RowLayout {
            width: parent.width - Theme.windowMargin
            anchors.rightMargin: 0
            anchors.right: parent.right
            spacing: Theme.windowMargin

            Image {
                source: "image://app/"+appID
                visible: appID !== ""
                sourceSize.width: Theme.windowMargin
                sourceSize.height: 6
                Layout.alignment: Qt.AlignTop
                Layout.leftMargin: 36
                Layout.topMargin: 4
                asynchronous: true
                fillMode:  Image.PreserveAspectFit
            }

            ColumnLayout {
                id: labelWrapper
                Layout.alignment: Qt.AlignTop
                spacing: 4

                VPNInterLabel {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    text: appName
                    color: Theme.fontColorDark
                    horizontalAlignment: Text.AlignLeft
                }

                VPNTextBlock {
                    Layout.fillWidth: true
                    text: appID
                    visible: !!text.length
                }

            }

            VPNCheckBox {
               Layout.alignment: Qt.AlignTop
               Layout.topMargin: 6
               onClicked: VPNAppPermissions.flip(appID)
               checked: appIsEnabled
               Layout.rightMargin: 4
            }
        }
    }
    VPNVerticalSpacer {
        Layout.preferredHeight: Theme.windowMargin * 3
        Layout.fillWidth: true
        visible: dividerVisible && isListVisible && applist.count > 0
        Rectangle {
            height: 1
            width: parent.width - Theme.windowMargin * 2
            color: "#e7e7e7"
            visible: isListVisible && applist.count > 0
            anchors.centerIn: parent
        }
    }

}
