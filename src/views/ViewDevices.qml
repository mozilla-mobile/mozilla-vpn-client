import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
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
            PropertyChanges {
                target: deviceLimitAlert
                visible: false
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
            PropertyChanges {
                target: deviceLimitAlert
                visible: true
            }
        }
    ]

    VPNMenu {
        id: menu
        title: qsTr("My devices")
    }

    VPNDeviceLimitAlert {
        id: deviceLimitAlert
        // TODO
    }

    ListView {
        id: deviceList
        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        clip: true
        interactive: false
        model: VPNDeviceModel

        delegate: Container {
            id: device
            width: parent.width

            contentItem: ColumnLayout {
                anchors.fill: parent
                spacing: 0

                RowLayout {
                    id: deviceRow
                    Layout.topMargin: Theme.vSpacing
                    Layout.leftMargin: Theme.windowMargin
                    Layout.rightMargin: Theme.windowMargin / 2
                    spacing: 0

                    VPNIcon {
                        source: "../resources/devices.svg"
                        fillMode: Image.PreserveAspectFit
                        Layout.rightMargin: Theme.windowMargin
                        Layout.alignment: Qt.AlignTop
                    }

                    ColumnLayout {
                        Layout.alignment: Qt.AlignTop
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

                    VPNIcon {
                        visible: !currentOne
                        source: "../resources/delete.svg"
                        Layout.rightMargin: Theme.windowMargin / 2
                        Layout.alignment: Qt.AlignTop

                        MouseArea {
                            anchors.fill: parent
                            onClicked: removePopup.initializeAndOpen(name)
                        }
                    }
                }
            }
        }
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
        anchors.centerIn: parent
        width: 260
        height: 240
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

        ColumnLayout {
            id: removePopupContent
            anchors.fill: removePopup
            anchors.centerIn: removePopup
            anchors.leftMargin: Theme.windowMargin
            anchors.rightMargin: Theme.windowMargin
            anchors.topMargin: 30
            anchors.bottomMargin: 30
            spacing: 0

            Rectangle {
                // placeholder until we have image
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: 64
                Layout.preferredWidth: 64
                Layout.bottomMargin: 20
            }

            Label {
                text: qsTr("Remove device?")
                Layout.alignment: Qt.AlignCenter
                Layout.bottomMargin: 10
                font.family: vpnFontInter.name
                font.weight: Font.Bold
                font.pixelSize: Theme.fontSizeSmall
                color: "#303030"
            }

            Text {
                text: qsTr("Please confirm you would like to remove\n%1.").arg(
                          removePopup.deviceName)
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                Layout.maximumWidth: 220
                Layout.bottomMargin: 16
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
