/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNClickableRow {
    id: serverCountry
    objectName: "serverCountry-" + code

    property bool cityListVisible: (code === focusScope.currentServer.countryCode)
    property real multiHopMenuHeight: VPNFeatureList.get("multiHop").isSupported ? VPNTheme.theme.menuHeight : 0
    property real animationDuration: 200 + (citiesRepeater.count * 25)
    property string _countryCode: code
    property var currentCityIndex
    property alias serverCountryName: countryName.text

    function openCityList() {
        cityListVisible = !cityListVisible;
        const itemDistanceFromWindowTop = serverCountry.mapToItem(null, 0, 0).y - multiHopMenuHeight;
        const listScrollPosition = vpnFlickable.contentY

        if (itemDistanceFromWindowTop + cityList.height < vpnFlickable.height || !cityListVisible) {
            return;
        }
        scrollAnimation.to = (cityList.height > vpnFlickable.height) ? listScrollPosition + itemDistanceFromWindowTop - VPNTheme.theme.rowHeight * 1.5 : listScrollPosition + cityList.height + VPNTheme.theme.rowHeight;
        scrollAnimation.duration = animationDuration
        scrollAnimation.start();
    }

    Keys.onReleased: event => {
        if (event.key === Qt.Key_Space) handleKeyClick()
    }

    onActiveFocusChanged: vpnFlickable.ensureVisible(serverCountry)
    handleMouseClick: openCityList
    handleKeyClick: openCityList
    clip: true

    activeFocusOnTab: true
    accessibleName: localizedName
    Keys.onDownPressed: countriesRepeater.itemAt(index + 1) ? countriesRepeater.itemAt(index + 1).forceActiveFocus() : countriesRepeater.itemAt(0).forceActiveFocus()
    Keys.onUpPressed: countriesRepeater.itemAt(index - 1) ? countriesRepeater.itemAt(index - 1).forceActiveFocus() : recentConnections.focusItemAt(recentConnections.numVisibleConnections - 1)
    Keys.onBacktabPressed: {
        serverSearchInput.forceActiveFocus();
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
                to: serverCountryRow.height + cityList.implicitHeight
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
        height: VPNTheme.theme.rowHeight
        width: parent.width

        VPNServerListToggle {
            id: serverListToggle

            Layout.leftMargin: VPNTheme.theme.windowMargin / 2
        }

        Image {
            id: flag

            source: "qrc:/nebula/resources/flags/" + code.toUpperCase() + ".png"
            fillMode: Image.PreserveAspectFit
            Layout.preferredWidth: VPNTheme.theme.iconSize
            Layout.preferredHeight: VPNTheme.theme.iconSize
            Layout.leftMargin: VPNTheme.theme.hSpacing
        }

        VPNBoldLabel {
            id: countryName

            text: localizedName
            Layout.leftMargin: VPNTheme.theme.hSpacing
            Layout.fillWidth: true
        }

    }

    Column {
        id: cityList
        objectName: "serverCityList"

        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: VPNTheme.theme.hSpacing + VPNTheme.theme.vSpacing + 6
        width: serverCountry.width - anchors.leftMargin

        Accessible.role: Accessible.List
        //% "Cities"
        //: The title for the list of cities.
        Accessible.name: qsTrId("cities")

        Repeater {
            id: citiesRepeater
            model: cities

            delegate: VPNRadioDelegate {
                property string _cityName: modelData.name
                property string _countryCode: code
                property string _localizedCityName: modelData.localizedName
                property string locationScore: VPNServerCountryModel.cityConnectionScore(code, modelData.code)
                property bool isAvailable: locationScore >= 0
                property int itemHeight: 54

                id: del
                objectName: "serverCity-" + del._cityName.replace(/ /g, '_')
                activeFocusOnTab: cityListVisible
                Keys.onDownPressed: if (citiesRepeater.itemAt(index + 1)) citiesRepeater.itemAt(index + 1).forceActiveFocus()
                Keys.onUpPressed: if (citiesRepeater.itemAt(index - 1)) citiesRepeater.itemAt(index - 1).forceActiveFocus()
                onActiveFocusChanged: vpnFlickable.ensureVisible(del)
                radioButtonLabelText: _localizedCityName
                accessibleName: _localizedCityName
                implicitWidth: parent.width

                onClicked: {
                    if (!isAvailable) {
                        return;
                    }

                    if (currentServer.whichHop === "singleHopServer") {
                        VPNController.changeServer(code, del._cityName);
                        return stackview.pop();
                    }

                    segmentedNav[currentServer.whichHop] = [del._countryCode,  del._cityName, del._localizedCityName]; // [countryCode, cityName, localizedCityName]
                    multiHopStackView.pop();
                }
                height: itemHeight
                checked: del._countryCode === focusScope.currentServer.countryCode && del._cityName === focusScope.currentServer.cityName
                isHoverable: cityListVisible && del.isAvailable
                enabled: cityListVisible && del.isAvailable

                Component.onCompleted: {
                    if (checked) {
                        currentCityIndex = index;
                    }
                }

                VPNIcon {
                    id: availabilityIndicator

                    anchors {
                        right: del.right
                        rightMargin: VPNTheme.theme.hSpacing
                        top: del.top
                        verticalCenter: del.verticalCenter
                    }
                    source: del.locationScore < 0 ? "qrc:/nebula/resources/warning.svg" :
                        "qrc:/nebula/resources/wifi-" + del.locationScore + ".svg"
                    sourceSize {
                        height: VPNTheme.theme.iconSizeSmall
                        width: VPNTheme.theme.iconSizeSmall
                    }
                    visible: del.locationScore != 0
                }

            }
        }

    }

}


