/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNClickableRow {
    id: serverCountry

    property bool cityListVisible: (code === VPNCurrentServer.countryCode)
    readonly property bool isActive: cityList.activeFocus
    property var currentCityIndex
    state: cityListVisible ? "list-visible" : "list-hidden"
    width: parent.width

    function openCityList() {
        cityListVisible = !cityListVisible;
        const itemDistanceFromWindowTop = serverCountry.mapToItem(null, 0, 0).y;
        const listScrollPosition = vpnFlickable.contentY

        if (itemDistanceFromWindowTop + cityList.height < vpnFlickable.height || !cityListVisible) {
            return;
        }
        scrollAnimation.to = (cityList.height > vpnFlickable.height) ? listScrollPosition + itemDistanceFromWindowTop - Theme.rowHeight * 1.5 : listScrollPosition + cityList.height + (Theme.windowMargin / 2);
        scrollAnimation.start();
    }

    Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
    handleMouseClick: openCityList
    handleKeyClick: openCityList
    clip: true

    activeFocusOnTab: true
    onActiveFocusChanged: parent.scrollDelegateIntoView(serverCountry)

    accessibleName: name

    states: [
        State {
            name: "list-hidden"

            PropertyChanges {
                target: serverCountry
                height: serverCountryRow.height
            }

            PropertyChanges {
                target: cityList
                opacity: 0
            }

        },
        State {
            name: "list-visible"

            PropertyChanges {
                target: serverCountry
                height: serverCountryRow.height + cityList.height
            }

            PropertyChanges {
                target: cityList
                opacity: 1
            }

        }
    ]
    // Override default VPNClickableRow transition.
    transitions: []

    Behavior on height {
        NumberAnimation {
            easing.type: Easing.InSine
            duration: 260
        }
    }

    function activate() {
        cityList.forceActiveFocus();
    }

    RowLayout {
        id: serverCountryRow

        spacing: 0
        height: Theme.rowHeight
        width: parent.width

        VPNServerListToggle {
            id: serverListToggle

            Layout.leftMargin: Theme.windowMargin / 2
        }

        Image {
            id: flag

            source: "../resources/flags/" + code.toUpperCase() + ".png"
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

    Column {

        id: cityList
        property var citySubList: "citySubList"
        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 22
        anchors.left: serverCountry.left
        anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 6
        opacity: 0
        width: serverCountry.width - anchors.leftMargin

        Repeater {
            model: cities
            delegate: VPNRadioDelegate {
                id: del

                activeFocusOnTab: cityListVisible
                onActiveFocusChanged: if (focus) serverList.scrollDelegateIntoView(del)
                property var isCurrentCity: modelData
                radioButtonLabelText: modelData
                accessibleName: modelData
                onClicked: {
                    VPNController.changeServer(code, modelData);
                    stackview.pop();
                }
                height: 54 // changes
                checked: code === VPNCurrentServer.countryCode && modelData === VPNCurrentServer.city
                isHoverable: serverCountry.cityListVisible
                enabled: cityListVisible
                Component.onCompleted: {
                    if (checked) {
                        currentCityIndex = index
                    }
                }

            }

        }

    }

}
