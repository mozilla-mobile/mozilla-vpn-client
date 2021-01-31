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
    property var currentCityIndex

    function openCityList() {
        cityListVisible = !cityListVisible;
        const itemDistanceFromWindowTop = serverCountry.mapToItem(null, 0, 0).y;
        const listScrollPosition = vpnFlickable.contentY

        if (itemDistanceFromWindowTop + cityList.height < vpnFlickable.height || !cityListVisible) {
            return;
        }
        scrollAnimation.to = (cityList.height > vpnFlickable.height) ? listScrollPosition + itemDistanceFromWindowTop - Theme.rowHeight * 1.5 : listScrollPosition + cityList.height + Theme.rowHeight;
        scrollAnimation.start();
    }

    Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
    handleMouseClick: openCityList
    handleKeyClick: openCityList
    clip: true

    activeFocusOnTab: true
    onActiveFocusChanged: parent.scrollDelegateIntoView(serverCountry)

    accessibleName: name
    Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).forceActiveFocus() : repeater.itemAt(0).forceActiveFocus()
    Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).forceActiveFocus() : menu.forceActiveFocus()
    Keys.onBacktabPressed: {
        focusScope.lastFocusedItemIdx = index;
        menu.forceActiveFocus();
    }

    state: cityListVisible
    states: [
        State {
            when: !cityListVisible

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
            when: cityListVisible

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

        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 22
        anchors.left: serverCountry.left
        anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 6
        width: serverCountry.width - anchors.leftMargin

        Accessible.role: Accessible.List
        //% "Cities"
        //: The title for the list of cities.
        Accessible.name: qsTrId("cities")

        Behavior on opacity {
            PropertyAnimation {
                duration: 300
            }
        }

        Repeater {
            id: citiesRepeater
            model: cities
            delegate: VPNRadioDelegate {
                id: del

                activeFocusOnTab: cityListVisible
                onActiveFocusChanged: if (focus) serverList.scrollDelegateIntoView(del)

                Keys.onDownPressed: if (citiesRepeater.itemAt(index + 1)) citiesRepeater.itemAt(index + 1).forceActiveFocus()
                Keys.onUpPressed: if (citiesRepeater.itemAt(index - 1)) citiesRepeater.itemAt(index - 1).forceActiveFocus()

                radioButtonLabelText: modelData
                accessibleName: modelData
                onClicked: {
                    VPNController.changeServer(code, modelData);
                    stackview.pop();
                }
                height: 54
                checked: code === VPNCurrentServer.countryCode && modelData === VPNCurrentServer.city
                isHoverable: cityListVisible
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
