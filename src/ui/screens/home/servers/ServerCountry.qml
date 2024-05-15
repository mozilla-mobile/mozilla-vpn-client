/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1
import compat 0.1

MZClickableRow {
    id: serverCountry
    objectName: "serverCountry-" + code

    property bool cityListVisible: (code === focusScope.currentServer.countryCode)
    property real multiHopMenuHeight: MZFeatureList.get("multiHop").isSupported ? MZTheme.theme.menuHeight : 0
    property int cityCount: cities.length
    property real animationDuration: 200 + (cityCount * 25)
    property string _countryCode: code
    property var currentCityIndex
    property alias serverCountryName: countryName.text
    property var cityList: cityListVisible ? cityLoader.item : cityLoader
    property var cityListExpandedHeight: 54 * cities.length
    property bool ready: cityListVisible && !(scrollAnimation.running || listOpenTransition.running || listClosedTransition.running)

    Component.onCompleted:{
       cityLoader.active = cityListVisible
    }

    // The city connection score can be used for every case except the multihop exit location,
    // where we need to use the scoring between the entry and exit locations instead.
    property bool useMultiHopScore: (focusScope.currentServer.whichHop === "multiHopExitServer")

    property bool hasAvailableCities: cities.reduce((initialValue, city) => (initialValue || city.connectionScore >= 0), false)

    function openCityList() {
        if(!cityListVisible){
            // If we are currently collapsed, 
            // sync load the list, before we show it. 
            cityLoader.active = true
        }
        cityListVisible = !cityListVisible;
        const itemDistanceFromWindowTop = serverCountry.mapToItem(null, 0, 0).y - multiHopMenuHeight;
        const listScrollPosition = vpnFlickable.contentY

        if (itemDistanceFromWindowTop + cityList.height < vpnFlickable.height || !cityListVisible) {
            return;
        }
        scrollAnimation.to = (cityList.height > vpnFlickable.height) ? listScrollPosition + itemDistanceFromWindowTop - MZTheme.theme.rowHeight * 1.5 : listScrollPosition + cityList.height + MZTheme.theme.rowHeight;
        scrollAnimation.duration = animationDuration
        scrollAnimation.start();
    }

    Keys.onReleased: event => {
        if (event.key === Qt.Key_Space) handleKeyClick()
    }

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
                height: serverCountryRow.height + cityListExpandedHeight
            }
            
            PropertyChanges {
                target: cityList
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            id: listClosedTransition
            to: "listClosed"
                SequentialAnimation{
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
                    ScriptAction{
                        // After we the close animation
                        // we can safely unload the list
                        script: {
                            cityLoader.active = false;
                        }
                    }
                }
        },
        Transition {
            id: listOpenTransition
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
        height: MZTheme.theme.rowHeight
        anchors.left: parent.left


        ServerListToggle {
            id: serverListToggle

            Layout.leftMargin: MZTheme.theme.windowMargin / 2
        }

        Image {
            id: flag

            source: "qrc:/ui/resources/flags/" + code.toUpperCase() + ".png"
            fillMode: Image.PreserveAspectFit
            Layout.preferredWidth: MZTheme.theme.iconSize
            Layout.preferredHeight: MZTheme.theme.iconSize
            Layout.leftMargin: MZTheme.theme.hSpacing
        }

        MZBoldLabel {
            id: countryName

            text: localizedName
            Layout.leftMargin: MZTheme.theme.hSpacing
            Layout.fillWidth: true
        }

    }

    Loader {
        id: cityLoader
        active: false
        asynchronous: false // Load sync, once we need that

        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 0
        anchors.left: parent.left
        anchors.leftMargin: MZTheme.theme.hSpacing + MZTheme.theme.vSpacing + 4
        width: serverCountry.width - anchors.leftMargin

  
        sourceComponent: Column {
            property alias count: citiesRepeater.count
            objectName: "serverCityList"

            Accessible.role: Accessible.List
            //% "Cities"
            //: The title for the list of cities.
            Accessible.name: qsTrId("cities")
            Accessible.ignored: !visible
            
            Repeater {
                id: citiesRepeater
                model: cities

                delegate: MZRadioDelegate {
                    property string _cityName: modelData.name
                    property string _countryCode: code
                    property string _localizedCityName: modelData.localizedName
                    property bool isAvailable: modelData.connectionScore >= 0
                    property int itemHeight: 54

                    id: del
                    objectName: "serverCity-" + del._cityName.replace(/ /g, '_')
                    activeFocusOnTab: cityListVisible
                    Keys.onDownPressed: if (citiesRepeater.itemAt(index + 1)) citiesRepeater.itemAt(index + 1).forceActiveFocus()
                    Keys.onUpPressed: if (citiesRepeater.itemAt(index - 1)) citiesRepeater.itemAt(index - 1).forceActiveFocus()
                    radioButtonLabelText: _localizedCityName
                    accessibleName: latencyIndicator.accessibleName.arg(_localizedCityName)
                    // Radio button label is ignored for Accesibility because accessibleName
                    // provides all the necessary information
                    isRadioButtonLabelAccessible: false
                    implicitWidth: parent.width

                    onClicked: {
                        if (!isAvailable) {
                            return;
                        }
                        focusScope.setSelectedServer(del._countryCode, del._cityName,del._localizedCityName);
                        MZSettings.recommendedServerSelected = false
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

                    ServerLatencyIndicator {
                        id: latencyIndicator
                        anchors {
                            right: parent.right
                            rightMargin: MZTheme.theme.hSpacing
                            verticalCenter: parent.verticalCenter
                        }
                        score: useMultiHopScore ? modelData.multiHopScore(segmentedNav.multiHopEntryServer[0], segmentedNav.multiHopEntryServer[1]) : modelData.connectionScore
                    }
                }
            }
        }
    }

}


