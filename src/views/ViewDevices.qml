/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: deviceWrapper
    state: VPNController.state !== VPNController.StateDeviceLimit ? "active" : "deviceLimit"

    states: [
        State {
            name: "active" // normal mode
            PropertyChanges {
                target: menu
                rightTitle: qsTr("%1 of %2").arg(
                                VPNDeviceModel.activeDevices).arg(
                                VPNUser.maxDevices)

            }
        },
        State {
            name: "deviceLimit" // device limit mode
            PropertyChanges {
                target: menu
                rightTitle: qsTr("%1 of %2").arg(
                                VPNDeviceModel.activeDevices + 1).arg(
                                VPNUser.maxDevices)
            }
        }
    ]

    VPNMenu {
        id: menu
        title: qsTr("My devices")
    }

    ListView {
        id: deviceList
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true
        interactive: true
        model: VPNDeviceModel
        spacing: 4

        delegate: Container {
            property var deviceName: name

            id: device
            width: deviceList.width

            Connections {
                target: VPN
                function onDeviceRemoving(devName) {
                    if (name === devName) {
                        deviceRemovalTransition.start();
                    }
                }
            }

            contentItem: ColumnLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    id: deviceRow
                    Layout.topMargin: 10
                    Layout.leftMargin: Theme.windowMargin
                    Layout.rightMargin: Theme.windowMargin / 2
                    spacing: 0

                    VPNIcon {
                        id: deviceIcon
                        source: "../resources/devices.svg"
                        fillMode: Image.PreserveAspectFit
                        Layout.rightMargin: Theme.windowMargin
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: Theme.windowMargin / 2
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: Theme.windowMargin / 2
                        spacing: 0

                        Text {
                            id: deviceName
                            text: name
                            color: Theme.fontColorDark
                            font.pixelSize: Theme.fontSize
                            font.family: vpnFontInter.name
                            Layout.preferredWidth: 220
                            Layout.preferredHeight: Theme.labelLineHeight
                            elide: Text.ElideRight
                        }

                        Text {
                            function deviceSubtitle() {
                                if (currentOne) {
                                    return qsTr("Current device")
                                }

                                const diff = (Date.now() - createdAt.valueOf(
                                                  )) / 1000

                                if (diff < 3600) {
                                    return qsTr("Added less than a hour ago")
                                }

                                if (diff < 86400) {
                                    return qsTr("Added %1 hours ago").arg(
                                                Math.floor(diff / 3600))
                                }

                                return qsTr("Added %1 days ago").arg(
                                            Math.floor(diff / 86400))
                            }
                            id: deviceDesc
                            text: deviceSubtitle()
                            Layout.preferredHeight: 21
                            font.pixelSize: Theme.fontSizeSmall
                            font.family: vpnFontInter.name
                            color: currentOne ? Theme.buttonColor : Theme.fontColor
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    VPNIconButton {
                        property var iconSource: "../resources/delete.svg"
                        property real iconHeightWidth: 22
                        property bool startRotation: false

                        id: iconButton
                        backgroundColor: Theme.removeDeviceBtn
                        visible: !currentOne
                        Layout.rightMargin: 0
                        Layout.topMargin: 0
                        Layout.alignment: Qt.AlignTop
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: 40
                        onClicked: removePopup.initializeAndOpen(name, index)

                        accessibleName: qsTr("Remove")

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
                                to: .6
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
                            property:  "iconSource"
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
            }
        }

        header: VPNDevicesListHeader {
            id: listHeader
        }

        footer: Rectangle {
            id: listfooter
            color: "transparent"
            height: 40
            width: parent.width
        }

        ScrollBar.vertical: ScrollBar {}
    }

    Rectangle {
        id: removePopupDarkBg
        anchors.fill: parent
        height: parent.height
        width: parent.width
        color: "#0c0c0d"
        opacity: removePopup.state === "visible" ? .25 : 0
        transitions: Transition {
            NumberAnimation {
                target: removePopupDarkBg
                property: "opacity"
                duration: 200
            }
        }
    }

    Rectangle {
        id: rectangularGlowClippingPath
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 96
        width: 260
        height: 232
        radius: 10
        opacity: removePopup.state === "visible" ? "1" : 0

        RectangularGlow {
            id: rectangularGlow
            anchors.fill: rectangularGlowClippingPath
            glowRadius: 10
            spread: .1
            color: "black"
            cornerRadius: rectangularGlowClippingPath.radius + glowRadius
            opacity: .4
        }
        transitions: Transition {
            NumberAnimation {
                target: rectangularGlowClippingPath
                property: "opacity"
                duration: 200
            }
        }
    }

    Rectangle {
        property var deviceName
        id: removePopup
        state: "invisible"
        anchors.fill: rectangularGlowClippingPath
        color: "#E1E1E1"
        radius: 10
        height: 232

        ColumnLayout {
            id: removePopupContent
            anchors.fill: removePopup
            anchors.centerIn: removePopup
            anchors.leftMargin: Theme.windowMargin
            anchors.rightMargin: Theme.windowMargin
            anchors.topMargin: 8
            anchors.bottomMargin: 30
            spacing: 0

            Image {
                Layout.alignment: Qt.AlignHCenter
                fillMode: Image.PreserveAspectFit
                source: "../resources/removeDevice.png"
                Layout.preferredHeight: 64
                Layout.preferredWidth: 64
                Layout.bottomMargin: 12
            }

            Text {
                text: qsTr("Remove device?")
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: Theme.fontSizeSmall
                font.weight: Font.Bold
                color: Theme.fontColorDark
            }

            Text {
                text: qsTr("Please confirm you would like to remove\n%1.").arg(
                          removePopup.deviceName)
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.maximumWidth: 220
                Layout.bottomMargin: 8
                font.family: vpnFontInter.name
                font.pixelSize: Theme.fontSizeSmallest
                lineHeightMode: Text.FixedHeight
                lineHeight: 14
                wrapMode: Text.Wrap
                color: "#262626"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                VPNPopupButton {
                    buttonText: qsTr("Cancel")
                    buttonTextColor: "#262626"
                    buttonColor: Theme.greyButton
                    onClicked: removePopup.state = "invisible"
                }

                VPNPopupButton {
                    id: removeBtn
                    buttonText: qsTr("Remove")
                    buttonTextColor: "#FFFFFF"
                    buttonColor: Theme.redButton
                    onClicked: {
                        VPN.removeDevice(removePopup.deviceName)
                        removePopup.state = "invisible"
                    }
                }
            }
        }

        states: [
            State {
                name: "invisible"
                PropertyChanges {
                    target: removePopup
                    opacity: 0
                }
            },

            State {
                name: "visible"
                PropertyChanges {
                    target: removePopup
                    opacity: 1.0
                }
            }
        ]

        transitions: Transition {
            NumberAnimation {
                properties: "opacity"
                duration: 100
            }
        }

        function initializeAndOpen(name) {
            removePopup.deviceName = name
            removePopup.state = "visible"
        }
    }
}
