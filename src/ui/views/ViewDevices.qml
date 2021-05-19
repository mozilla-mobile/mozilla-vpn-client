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
        objectName: "deviceListBackButton"

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
        flickContentHeight: maxDevicesReached.height + content.height + col.height
        contentHeight: maxDevicesReached.height + content.height + col.height
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

            width: root.width
        }

        ColumnLayout {
            id: content
            width: vpnFlickable.width
            anchors.top: maxDevicesReached.bottom
            anchors.topMargin: Theme.windowMargin / 2
            spacing: Theme.windowMargin / 2

            Repeater {
                id: deviceList
                model: VPNDeviceModel
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true

                delegate: VPNDeviceListItem{}
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: 1
            }

            ColumnLayout {
                id: col
                spacing: 0

                Layout.preferredWidth: vpnFlickable.width
                Layout.alignment: Qt.AlignHCenter

                VPNVerticalSpacer {
                    Layout.preferredHeight: Theme.windowMargin * 2
                    Layout.preferredWidth: vpnFlickable.width - Theme.windowMargin * 2
                    Layout.alignment: Qt.AlignHCenter
                    Rectangle {
                        id: divider
                        height: 1
                        width:  parent.width
                        anchors.centerIn: parent
                        color: "#e7e7e7"
                    }
                }

                VPNLinkButton {
                    id: getHelpLink

                    labelText: qsTrId("vpn.main.getHelp")
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: stackview.push("../views/ViewGetHelp.qml")
                    Layout.preferredHeight: Theme.rowHeight
                }

                VPNSignOut {
                    id: signOff
                    anchors.bottom: undefined
                    anchors.horizontalCenter: undefined
                    anchors.bottomMargin: undefined
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: Theme.rowHeight
                    onClicked: {
                        VPNController.logout();
                    }
                }
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
