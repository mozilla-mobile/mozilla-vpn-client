/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNViewBase {
    property var isModalDialogOpened: removePopup.visible
    property var wasmView

    id: vpnFlickable

    state: VPN.state !== VPN.StateDeviceLimit ? "active" : "deviceLimit"

    //% "My devices"
    _menuTitle: qsTrId("vpn.devices.myDevices")
    _viewContentData: ColumnLayout {
        id: content

        objectName: "deviceListView"
        Layout.fillWidth: true
        spacing: VPNTheme.theme.windowMargin

        VPNDevicesListHeader {
            id: maxDevicesReached
            Layout.fillWidth: true
            visible: VPN.state === VPN.StateDeviceLimit
        }

        Repeater {
            id: deviceList
            model: VPNDeviceModel
            Layout.fillWidth: true
            delegate: VPNDeviceListItem { objectName: "device-" + name }
        }

        ColumnLayout {
            id: col
            Layout.fillWidth: true

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
                Layout.fillWidth: true

                Rectangle {
                    id: divider
                    height: 1
                    anchors {
                        left: parent.left
                        right: parent.right
                        leftMargin: VPNTheme.theme.windowMargin
                        rightMargin: VPNTheme.theme.windowMargin
                        verticalCenter: parent.verticalCenter
                    }
                    color: "#e7e7e7"
                }
            }

            VPNLinkButton {
                id: getHelpLink

                labelText: qsTrId("vpn.main.getHelp2")
                Layout.alignment: Qt.AlignHCenter
                onClicked: VPNNavigator.requestScreen(VPNNavigator.ScreenGetHelp)
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
    VPNRemoveDevicePopup {
        id: removePopup

        Connections {
            target: VPN
            function onDeviceRemoving(devPublicKey) {
                if(VPN.state === VPN.StateDeviceLimit) {
                    for(var i = 0; i < deviceList.count; i++) {
                        deviceList.itemAt(i).enabled = false
                    }
                }
            }
        }

        function initializeAndOpen(name, publicKey) {
            removePopup.deviceName = name;
            removePopup.devicePublicKey = publicKey;
            removePopup.open();
        }
    }

    Component.onCompleted: {
        VPN.refreshDevices()
        if (wasmView) {
            state = "deviceLimit"
        }
    }

    states: [
        State {
            name: "active" // normal mode

            PropertyChanges {
                target: menu
                //% "%1 of %2"
                //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
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
                //% "%1 of %2"
                //: Example: You have "x of y" devices in yor account, where y is the limit of allowed devices.
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

    Component.onDestruction: menu.rightTitle = ""
}
