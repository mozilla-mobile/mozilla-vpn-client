import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    width: parent.width

    VPNMenu {
        id: menu
        title: qsTr("Select location")
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    ListView {
        id: serverList

        height: parent.height - menu.height
        width: parent.width

        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.topMargin: Theme.vSpacing
        anchors.leftMargin: Theme.hSpacing
        anchors.rightMargin: Theme.hSpacing

        clip: true
        model: VPN.serverCountryModel

        delegate: ColumnLayout {
            property bool cityListVisible: false

            id: serverCountry
            spacing: 0
            state: cityListVisible ? "list-visible" : "list-hidden"
            width: serverList.width

            RowLayout {
                Layout.preferredWidth: parent.width
                Layout.bottomMargin: Theme.vSpacing
                spacing: 0

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    width: parent.width
                    onClicked: cityListVisible = !cityListVisible
                    hoverEnabled: true
                }

                ServerListToggle {
                    id: serverListToggle
                    states: State {
                        name: "rotated"
                        when: serverCountry.cityListVisible
                        PropertyChanges {
                            target: serverListToggle
                            rotation: 90
                        }
                    }
                }

                Image {
                    id: flag
                    source: "../resources/flags/" + code.toUpperCase() + ".png"
                    fillMode: Image.PreserveAspectFit
                    Layout.preferredWidth: Theme.iconSize
                    Layout.preferredHeight: Theme.iconSize
                    Layout.leftMargin: Theme.hSpacing
                }

                Label {
                    id: countryName
                    text: name
                    Layout.leftMargin: Theme.hSpacing
                    Layout.fillWidth: true

                    color: Theme.fontColorDark
                    font.family: vpnFont.name
                    font.pixelSize: Theme.fontSize
                    font.weight: Font.Bold
                    height: Theme.labelLineHeight

                    states: State {
                        when: mouseArea.containsMouse
                        PropertyChanges {
                            target: countryName
                            opacity: .7
                        }
                    }

                    transitions: Transition {
                        NumberAnimation {
                            target: countryName
                            properties: "opacity"
                            duration: 100
                        }
                    }
                }
            }

            Item {
                id: cityListWrapper

                Layout.preferredHeight: cityList.contentHeight
                Layout.fillWidth: true

                ListView {
                    id: cityList
                    anchors.fill: parent
                    Layout.fillWidth: true

                    model: cities
                    delegate: RadioDelegate {
                        id: control
                        ButtonGroup.group: radioButtonGroup

                        width: parent.width

                        Layout.fillWidth: true
                        Layout.bottomMargin: Theme.vSpacing

                        indicator: RadioButton {
                            id: radioButton
                        }

                        RadioButtonLabel {
                            id: cityName
                        }

                        MouseArea {
                            id: radioButtonMouseArea
                            anchors.fill: parent
                            onClicked: control.checked = !control.checked
                        }
                    }
                }
            }

            // Transitioning opacity and Layout.maximumHeight properties
            // since the `visible` property cannot be transitioned
            // smoothly. Might be cleaner ways to do this.
            states: [
                State {
                    name: "list-hidden"
                    PropertyChanges {
                        target: cityListWrapper
                        opacity: 0
                        Layout.maximumHeight: 0
                    }
                },

                State {
                    name: "list-visible"
                    PropertyChanges {
                        target: cityListWrapper
                        opacity: 1
                        Layout.maximumHeight: cityList.contentHeight
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation {
                    properties: "opacity, Layout.maximumHeight"
                    easing.type: Easing.InSine
                    duration: 200
                }
            }
        }
        populate: Transition {
            NumberAnimation {
                from: 0
                to: 1
                property: "opacity"
                duration: 300
            }
        }
    }
}
