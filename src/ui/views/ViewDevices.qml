/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    id: root

    property var isModalDialogOpened: removePopup.visible
    property var wasmView

    VPNMenu {
        id: menu
        objectName: "deviceListBackButton"

        //% "My devices"
        title: qsTrId("vpn.devices.myDevices")
        accessibleIgnored: isModalDialogOpened
    }

    VPNFlickable {
        id: vpnFlickable

        anchors.top: menu.bottom
        height: root.height - menu.height
        anchors.left: root.left
        anchors.right: root.right
        interactive: true
        flickContentHeight: maxDevicesReached.height + content.height + col.height
        contentHeight: maxDevicesReached.height + content.height + col.height
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
                PropertyChanges {
                    target: col
                    visible: false
                }
                PropertyChanges {
                    target: menu
                    btnDisabled: false
                }
            },
            State {
                name: "deviceLimit"

                PropertyChanges {
                    target: menu
                    rightTitle: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + 1).arg(VPNUser.maxDevices)
                }
                PropertyChanges {
                    target: col
                    visible: true
                }
                PropertyChanges {
                    target: menu
                    btnDisabled: true
                }
            }
        ]

        VPNDevicesListHeader {
            id: maxDevicesReached

            anchors.left: parent.left
            anchors.right: parent.right
        }

        Column {
            id: content
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: maxDevicesReached.bottom
            anchors.topMargin: 0
            spacing: VPNTheme.theme.windowMargin / 2


            Repeater {
                id: deviceList
                model: VPNDeviceModel
                anchors.left: parent.left
                anchors.right: parent.right
                delegate: VPNDeviceListItem {}
            }


            VPNVerticalSpacer {
                Layout.preferredHeight: 1
            }

            ColumnLayout {
                id: col
                anchors.horizontalCenter: parent.horizontalCenter

                VPNVerticalSpacer {
                    Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
                    Layout.preferredWidth: vpnFlickable.width - VPNTheme.theme.windowMargin * 2
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    Rectangle {
                        id: divider
                        height: 1
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.centerIn: parent
                        color: "#e7e7e7"
                    }
                }

                VPNLinkButton {
                    id: getHelpLink

                    labelText: qsTrId("vpn.main.getHelp2")
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: getHelpViewNeeded()
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                }

                VPNSignOut {
                    id: signOff
                    anchors.bottom: undefined
                    anchors.horizontalCenter: undefined
                    anchors.bottomMargin: undefined
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                }
            }
        }
    }

    VPNRemoveDevicePopup {
        id: removePopup

        function initializeAndOpen(name, publicKey) {
            removePopup.deviceName = name;
            removePopup.devicePublicKey = publicKey;
            removePopup.open();
        }
    }

    Component.onCompleted: VPN.refreshDevices()
}
