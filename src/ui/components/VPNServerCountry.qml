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
    objectName: "serverCountry-" + code

    property bool cityListVisible: (code === VPNCurrentServer.countryCode)
    property real animationDuration: 200 + (citiesRepeater.count * 25)
    property var currentCityIndex
    property alias serverCountryName: countryName.text

    function openCityList() {
        cityListVisible = !cityListVisible;
        const itemDistanceFromWindowTop = serverCountry.mapToItem(null, 0, 0).y;
        const listScrollPosition = vpnFlickable.contentY

        if (itemDistanceFromWindowTop + cityList.height < vpnFlickable.height || !cityListVisible) {
            return;
        }
        scrollAnimation.to = (cityList.height > vpnFlickable.height) ? listScrollPosition + itemDistanceFromWindowTop - Theme.rowHeight * 1.5 : listScrollPosition + cityList.height + Theme.rowHeight;
        scrollAnimation.duration = animationDuration
        scrollAnimation.start();
    }

    Keys.onReleased: if (event.key === Qt.Key_Space) handleKeyClick()
    handleMouseClick: openCityList
    handleKeyClick: openCityList
    clip: true

    activeFocusOnTab: true
    accessibleName: localizedName
    Keys.onDownPressed: repeater.itemAt(index + 1) ? repeater.itemAt(index + 1).forceActiveFocus() : repeater.itemAt(0).forceActiveFocus()
    Keys.onUpPressed: repeater.itemAt(index - 1) ? repeater.itemAt(index - 1).forceActiveFocus() : menu.forceActiveFocus()
    Keys.onBacktabPressed: {
        focusScope.lastFocusedItemIdx = index;
        menu.forceActiveFocus();
    }

    state: cityListVisible ? "listOpen" : "listClosed"
    states: [
        State {
            name: "listClosed"

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
            name: "listOpen"

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

    transitions: [
        Transition {
            to: "listClosed"
            ParallelAnimation {
                PropertyAnimation {
                    target: cityList
                    property: "opacity"
                    duration: animationDuration
                }
                PropertyAnimation {
                    target: serverCountry
                    property: "height"
                    duration: animationDuration
                }
            }
        },
        Transition {
            to: "listOpen"
            PropertyAnimation {
                target: serverCountry
                property: "height"
                to: serverCountryRow.height + cityList.height
                duration: animationDuration
            }
            PropertyAnimation {
                target: cityList
                property: "opacity"
                duration: 0
            }
        }

    ]

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

            text: localizedName
            Layout.leftMargin: Theme.hSpacing
            Layout.fillWidth: true
        }

    }

    Column {
        id: cityList
        objectName: "serverCityList"

        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 22
        anchors.left: serverCountry.left
        anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 6
        width: serverCountry.width - anchors.leftMargin

        Accessible.role: Accessible.List
        //% "Cities"
        //: The title for the list of cities.
        Accessible.name: qsTrId("cities")

        Repeater {
            id: citiesRepeater
            model: cities
            delegate: VPNRadioDelegate {
                id: del
                objectName: "serverCity-" + modelData[0].replace(/ /g, '_')

                activeFocusOnTab: cityListVisible

                Keys.onDownPressed: if (citiesRepeater.itemAt(index + 1)) citiesRepeater.itemAt(index + 1).forceActiveFocus()
                Keys.onUpPressed: if (citiesRepeater.itemAt(index - 1)) citiesRepeater.itemAt(index - 1).forceActiveFocus()

                onActiveFocusChanged: if (focus) vpnFlickable.ensureVisible(del)

                radioButtonLabelText: modelData[1]
                accessibleName: modelData[1]
                onClicked: {
                    VPNController.changeServer(code, modelData[0]);
                    stackview.pop();
                }
                height: 54
                checked: code === VPNCurrentServer.countryCode && modelData[0] === VPNCurrentServer.cityName
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
