/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: device

    Layout.fillWidth: true
    Layout.preferredHeight: deviceRow.implicitHeight + VPNTheme.theme.windowMargin
    //% "%1 %2"
    //: Example: "deviceName deviceDescription"
    Accessible.name: qsTrId("vpn.devices.deviceAccessibleName").arg(name).arg(deviceDesc.text)
    Accessible.role: Accessible.ListItem
    Accessible.ignored: isModalDialogOpened
    activeFocusOnTab: true
    onActiveFocusChanged: if (focus) vpnFlickable.ensureVisible(device)

    Rectangle {
        color: VPNTheme.theme.greyHovered
        opacity: device.focus || iconButton.focus ? 1 : 0
        anchors.fill: device
        anchors.topMargin: -8
        anchors.bottomMargin: -20

        Behavior on opacity {
            PropertyAnimation {
                duration: 50
            }
        }
    }

    RowLayout {
        property var deviceName: name
        objectName: "deviceLayout"

        id: deviceRow

        spacing: 0
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.rightMargin: VPNTheme.theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: VPNTheme.theme.windowMargin

        SequentialAnimation {
            id: deviceRemovalTransition

            ParallelAnimation {
                PropertyAnimation {
                    target: iconButton
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 100
                }

                PropertyAnimation {
                    targets: [deviceName, deviceDesc, deviceIcon]
                    property: "opacity"
                    from: 1
                    to: 0.6
                    duration: 100
                }

            }

            PropertyAction {
                target: iconButton
                property: "iconHeightWidth"
                value: 20
            }

            PropertyAction {
                target: iconButton
                property: "iconSource"
                value: "qrc:/nebula/resources/spinner.svg"
            }

            ScriptAction { script: icon.rotating = true; }

            ParallelAnimation {
                PropertyAnimation {
                    target: iconButton
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                }
            }
        }


        Connections {
            function onDeviceRemoving(devPublicKey) {
                if (publicKey === devPublicKey)
                    deviceRemovalTransition.start();
            }

            target: VPN
        }

        VPNIcon {
            id: deviceIcon

            source: "qrc:/ui/resources/devices.svg"
            fillMode: Image.PreserveAspectFit
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
            opacity: enabled ? 1 : 0.6
        }

        Column {
            id: deviceInfo

            Layout.fillWidth: true
            spacing: 0

            VPNInterLabel {
                id: deviceName

                text: name
                color: VPNTheme.theme.fontColorDark
                elide: Text.ElideRight
                anchors.left: parent.left
                horizontalAlignment: Text.AlignLeft
                anchors.right: parent.right
                opacity: enabled ? 1 : 0.6
                Accessible.ignored: isModalDialogOpened
            }

            VPNTextBlock {
                id: deviceDesc

                function deviceSubtitle() {
                    if (currentOne) {
                        //% "Current Device"
                        return qsTrId("vpn.devices.currentDevice");
                    }

                    const diff = (Date.now() - createdAt.valueOf()) / 1000;
                    if (diff < 3600) {
                        //% "Added less than an hour ago"
                        return qsTrId("vpn.devices.addedltHour");
                    }

                    if (diff < 86400) {
                        //: %1 is the number of hours.
                        //% "Added a few hours ago (%1)"
                        return qsTrId("vpn.devices.addedXhoursAgo").arg(Math.floor(diff / 3600));
                    }

                    //% "Added %1 days ago"
                    //: %1 is the number of days.
                    //: Note: there is currently no support for proper plurals
                    return qsTrId("vpn.devices.addedXdaysAgo").arg(Math.floor(diff / 86400));
                }

                text: deviceSubtitle()
                width: parent.width
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                verticalAlignment: Text.AlignVCenter
                opacity: enabled ? 1 : 0.6

            }
        }

        VPNIconButton {
            id: iconButton
            objectName: "deviceRemoveButton"

            property var iconSource: "qrc:/nebula/resources/delete.svg"
            property real iconHeightWidth: 22

            buttonColorScheme: VPNTheme.theme.removeDeviceBtn
            visible: !currentOne
            opacity: enabled || (!enabled && iconSource === "qrc:/nebula/resources/spinner.svg") ? 1 : 0.6
            Layout.topMargin: -8
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.preferredWidth: VPNTheme.theme.rowHeight
            onClicked: removePopup.initializeAndOpen(name, publicKey)
            //: Label used for accessibility on the button to remove a device. %1 is the name of the device.
            //% "Remove %1"
            accessibleName: qsTrId("vpn.devices.removeA11Y").arg(deviceRow.deviceName)
            // Only allow focus within the current item in the list.

            Accessible.ignored: isModalDialogOpened

            VPNIcon {
                id: icon

                property bool rotating: false

                source: iconButton.iconSource
                anchors.centerIn: iconButton
                sourceSize.height: iconButton.iconHeightWidth
                sourceSize.width: iconButton.iconHeightWidth

                PropertyAnimation {
                    target: icon
                    running: icon.rotating
                    property: "rotation"
                    from: 0
                    to: 360
                    duration: 5000
                    loops: Animation.Infinite
                }

            }

        }
    }

}
