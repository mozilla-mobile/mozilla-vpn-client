/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import themes 0.1

Item {
    id: device

    anchors.left: parent.left
    anchors.right: parent.right
    height: deviceRow.implicitHeight + Theme.windowMargin
    //% "%1 %2"
    //: Example: "deviceName deviceDescription"
    Accessible.name: qsTrId("vpn.devices.deviceAccessibleName").arg(name).arg(deviceDesc.text)
    Accessible.role: Accessible.ListItem
    Accessible.ignored: root.isModalDialogOpened
    activeFocusOnTab: true
    onActiveFocusChanged: if (focus) vpnFlickable.ensureVisible(device)

    Rectangle {
        color: Theme.greyHovered
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

        id: deviceRow

        spacing: 0
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.rightMargin: Theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin

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
                value: "qrc:/ui/resources/spinner.svg"
            }

            ParallelAnimation {
                PropertyAnimation {
                    target: iconButton
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                }

                PropertyAction {
                    target: iconButton
                    property: "startRotation"
                    value: true
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
            Layout.leftMargin: Theme.windowMargin
            Layout.rightMargin: Theme.windowMargin
            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
        }

        Column {
            id: deviceInfo

            Layout.fillWidth: true
            spacing: 0

            VPNInterLabel {
                id: deviceName

                text: name
                color: Theme.fontColorDark
                elide: Text.ElideRight
                anchors.left: parent.left
                horizontalAlignment: Text.AlignLeft
                anchors.right: parent.right
                Accessible.ignored: root.isModalDialogOpened
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

            }
        }

        VPNIconButton {
            id: iconButton

            property var iconSource: "qrc:/ui/resources/delete.svg"
            property real iconHeightWidth: 22
            property bool startRotation: false

            buttonColorScheme: Theme.removeDeviceBtn
            visible: !currentOne
            Layout.topMargin: -8
            Layout.alignment: Qt.AlignTop | Qt.AlignRight
            Layout.preferredHeight: Theme.rowHeight
            Layout.preferredWidth: Theme.rowHeight
            onClicked: removePopup.initializeAndOpen(name, publicKey)
            //: Label used for accessibility on the button to remove a device. %1 is the name of the device.
            //% "Remove %1"
            accessibleName: qsTrId("vpn.devices.removeA11Y").arg(deviceRow.deviceName)
            // Only allow focus within the current item in the list.

            Accessible.ignored: root.isModalDialogOpened

            VPNIcon {
                source: iconButton.iconSource
                anchors.centerIn: iconButton
                sourceSize.height: iconButton.iconHeightWidth
                sourceSize.width: iconButton.iconHeightWidth
                rotation: iconButton.startRotation ? 360 : 0

                Behavior on rotation {
                    PropertyAnimation {
                        duration: 5000
                        loops: Animation.Infinite
                    }

                }

            }

        }
    }

}
