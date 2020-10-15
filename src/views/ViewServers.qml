/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNMenu {
        id: menu
        title: qsTrId("selectLocation")
    }

    ButtonGroup {
        id: radioButtonGroup
    }

    ListView {
        id: serverList

        height: parent.height - menu.height
        width: parent.width
        anchors.top: menu.bottom
        spacing: Theme.listSpacing

        clip: true
        model: VPNServerCountryModel
        header: Rectangle {
            height: 16
            width: serverList.width
            color: "transparent"
        }

        delegate: Item {
            property var cityListVisible: false
            id: serverCountry
            state: cityListVisible ? "list-visible" : "list-hidden"
            width: serverList.width
            height: 40

            VPNClickableRow {
                id: serverCountryRow
                onClicked: cityListVisible = !cityListVisible

                accessibleName: name

                RowLayout {
                    spacing: 0
                    height: 40
                    anchors.fill: serverCountryRow
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8

                    VPNServerListToggle {
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

                    VPNBoldLabel {
                        id: countryName
                        text: name
                        Layout.leftMargin: Theme.hSpacing
                        Layout.fillWidth: true
                    }
                }
            }

            Item {
                id: cityListWrapper
                anchors.top: serverCountryRow.bottom
                width: parent.width

                ListView {
                    property var listLength: cities.length

                    id: cityList
                    interactive: false
                    model: cities
                    spacing: Theme.listSpacing
                    width: parent.width
                    anchors.fill: parent
                    anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 14
                    delegate: VPNRadioDelegate {
                        radioButtonLabelText: modelData
                        onClicked: {
                            VPNController.changeServer(code, modelData)
                            stackview.pop()
                        }
                        checked: code === VPNCurrentServer.countryCode
                                 && modelData === VPNCurrentServer.city
                        isHoverable: cityListVisible
                    }
                }
            }

            states: [
                State {
                    name: "list-hidden"
                    PropertyChanges {
                        target: cityListWrapper
                        opacity: 0
                        height: 0
                    }
                },

                State {
                    name: "list-visible"
                    PropertyChanges {
                        target: cityListWrapper
                        opacity: 1
                        anchors.topMargin: 18
                        height: (cityList.listLength * 48)
                    }
                    PropertyChanges {
                        target: serverCountry
                        height: cityListWrapper.height + 38
                    }
                }
            ]

            transitions: Transition {
                NumberAnimation {
                    properties: "opacity, height, anchors.topMargin"
                    easing.type: Easing.InSine
                    duration: 200
                }
            }
        }

        ScrollBar.vertical: ScrollBar {}
    }
}
