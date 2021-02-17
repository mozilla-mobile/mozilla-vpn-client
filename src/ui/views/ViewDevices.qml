/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: root

    property var isModalDialogOpened: removePopup.visible
    property var wasmView
    height: window.safeContentHeight
    width: window.width

    VPNMenu {
        id: menu

        //% "My devices"
        title: qsTrId("vpn.devices.myDevices")
        accessibleIgnored: isModalDialogOpened
    }

    VPNFlickable {
        id: vpnFlickable

        anchors.top: menu.bottom
        height: root.height - menu.height
        width: root.width
        interactive: true
        flickContentHeight: maxDevicesReached.height + deviceList.height
        contentHeight: maxDevicesReached.height + deviceList.height
        contentWidth: window.width
        state: VPN.state !== VPN.StateDeviceLimit ? "active" : "deviceLimit"
        Component.onCompleted: {
            if (wasmView) {
                state = "deviceLimit"
            }
        }
        states: [
            State {
                name: "active" // normal mode

                PropertyChanges {
                    target: menu
                    rightTitle: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices).arg(VPNUser.maxDevices)
                }

            },
            State {
                name: "deviceLimit"

                PropertyChanges {
                    target: menu
                    rightTitle: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + 1).arg(VPNUser.maxDevices)
                }

            }
        ]

        VPNDevicesListHeader {
            id: maxDevicesReached

            width: root.width
        }

        VPNList {
            id: deviceList

            property VPNIconButton focusedIconButton: null

            height: count * 80
            width: parent.width
            anchors.top: maxDevicesReached.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            interactive: false
            model: VPNDeviceModel
            spacing: 20
            Keys.onDownPressed: {
                vpnFlickable.ensureVisible(currentItem);
            }
            Keys.onUpPressed: {
                vpnFlickable.ensureVisible(currentItem);
            }
            onFocusChanged: {
                // Clear focus from the last focused remove button.
                if (deviceList.focusedIconButton) {
                    deviceList.focusedIconButton.focus = false;
                    deviceList.focusedIconButton = null;
                }
            }
            listName: menu.title
            Accessible.ignored: root.isModalDialogOpened

            highlightFollowsCurrentItem: false
            highlight: Rectangle {
                color: Theme.greyHovered
                width: deviceList.width
                height: deviceList.currentItem ? deviceList.currentItem.height + 20 : 0
                y: deviceList.currentItem ? deviceList.currentItem.y - 10 : 0
                opacity: deviceList.currentItem && deviceList.currentItem.activeFocus ? 1 : 0

                Behavior on y {
                    PropertyAnimation {
                        duration: 10
                    }
                }
            }

            removeDisplaced: Transition {
                NumberAnimation {
                    properties: "x,y"
                    duration: Theme.removeDeviceAnimation
                }

            }

            remove: Transition {
                ParallelAnimation {
                    NumberAnimation {
                        property: "opacity"
                        to: 0
                        duration: Theme.removeDeviceAnimation
                    }

                }

            }

            delegate: RowLayout {
                id: deviceRow

                property var deviceName: name

                spacing: 0
                //% "%1 %2"
                //: Example: "deviceName deviceDescription"
                Accessible.name: qsTrId("vpn.devices.deviceAccessibleName").arg(name).arg(deviceDesc.text)
                Accessible.role: Accessible.ListItem
                Accessible.ignored: root.isModalDialogOpened
                width: deviceList.width - Theme.windowMargin
                anchors.horizontalCenter: parent.horizontalCenter

                Connections {
                    function onDeviceRemoving(devName) {
                        if (name === devName)
                            deviceRemovalTransition.start();

                    }

                    target: VPN
                }

                VPNIcon {
                    id: deviceIcon

                    source: "../resources/devices.svg"
                    fillMode: Image.PreserveAspectFit
                    Layout.leftMargin: Theme.windowMargin / 2
                    Layout.rightMargin: Theme.windowMargin
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.topMargin: Theme.windowMargin / 2
                }

                ColumnLayout {
                    id: deviceInfo

                    Layout.alignment: Qt.AlignLeft
                    Layout.topMargin: Theme.windowMargin / 2
                    Layout.preferredWidth: deviceRow.Layout.preferredWidth - deviceIcon.Layout.rightMargin - deviceIcon.sourceSize.width - iconButton.width - Theme.windowMargin
                    spacing: 0

                    VPNInterLabel {
                        id: deviceName

                        text: name
                        color: Theme.fontColorDark
                        elide: Text.ElideRight
                        Layout.alignment: Qt.AlignLeft
                        horizontalAlignment: Text.AlignLeft
                        Layout.preferredWidth: deviceInfo.Layout.preferredWidth - Theme.windowMargin / 2
                        Layout.fillWidth: true
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
                        color: currentOne ? Theme.blue : Theme.fontColor
                        Accessible.ignored: root.isModalDialogOpened
                    }

                }

                VPNIconButton {
                    id: iconButton

                    property var iconSource: "../resources/delete.svg"
                    property real iconHeightWidth: 22
                    property bool startRotation: false

                    buttonColorScheme: Theme.removeDeviceBtn
                    visible: !currentOne
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    Layout.preferredHeight: Theme.rowHeight
                    Layout.preferredWidth: Theme.rowHeight
                    onClicked: removePopup.initializeAndOpen(name, index)
                    //: Label used for accessibility on the button to remove a device. %1 is the name of the device.
                    //% "Remove %1"
                    accessibleName: qsTrId("vpn.devices.removeA11Y").arg(deviceRow.deviceName)
                    // Only allow focus within the current item in the list.
                    focusPolicy: deviceList.currentItem === deviceRow ? Qt.StrongFocus : Qt.NoFocus
                    onFocusChanged: {
                        // If the remove button gets a focus, remember it.
                        if (focus)
                            deviceList.focusedIconButton = this;

                    }
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
                        value: "../resources/spinner.svg"
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

            }

            footer: Rectangle {
                id: listfooter

                color: "transparent"
                height: 40
                width: parent.width
            }

        }

    }

    VPNRemoveDevicePopup {
        id: removePopup

        function initializeAndOpen(name) {
            removePopup.deviceName = name;
            removePopup.open();
        }

    }

    Component.onCompleted: VPN.refreshDevices()
}
