import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
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

        anchors.leftMargin: Theme.hSpacing
        anchors.rightMargin: Theme.hSpacing

        clip: true
        model: VPNServerCountryModel

        delegate: ColumnLayout {
            property var cityListVisible: false

            id: serverCountry
            spacing: 0
            state: cityListVisible ? "list-visible" : "list-hidden"
            width: serverList.width

            ItemDelegate {
                id: serverCountryRow
                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 22
                Layout.topMargin: Theme.vSpacing / 2
                Layout.bottomMargin: serverCountryRow.Layout.topMargin
                spacing: 0

                MouseArea {
                    anchors.fill: serverCountryRow
                    onClicked: cityListVisible = !cityListVisible
                }

                RowLayout {
                    spacing: 0
                    ServerListToggle {
                        id: serverListToggle
                    }

                    Image {
                        id: flag
                        source: "../resources/flags/" + code.toUpperCase(
                                    ) + ".png"
                        fillMode: Image.PreserveAspectFit
                        Layout.preferredWidth: Theme.iconSize
                        Layout.preferredHeight: Theme.iconSize
                        Layout.leftMargin: Theme.hSpacing
                    }

                    BoldLabel {
                        id: countryName
                        text: name
                        Layout.leftMargin: Theme.hSpacing
                        Layout.fillWidth: true
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
                        checked: code == VPNCurrentServer.countryCode && cityName.text == VPNCurrentServer.city

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
                            onClicked: {
                                VPNController.changeServer(code, cityName.text);
                                stackview.pop();
                            }
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
    }
}
