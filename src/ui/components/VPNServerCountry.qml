/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNClickableRow {
    id: serverCountry

    property var cityListVisible: false
    readonly property bool isActive: cityList.activeFocus

    // During transition, when expanding the row or scrolling,
    // delegate parent might be null and thus trying to access
    // parent.left[right] triggers a warning. We can temporarily
    // set the values to undefined as they are going to be
    // overridden once the delegate parent is set again.
    anchors.left: parent ? parent.left : undefined
    anchors.right: parent ? parent.right : undefined
    state: cityListVisible ? "list-visible" : "list-hidden"
    width: ListView.view.width
    onClicked: cityListVisible = !cityListVisible
    accessibleName: name
    anchorToFill: serverCountryRow
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
            duration: 200
        }
    }
    Keys.onDownPressed: {
        if (!cityListVisible) {
            event.accepted = false;
            return;
        }

        if (cityList.activeFocus) {
            event.accepted = false;
        } else {
            cityList.forceActiveFocus();
        }
    }
    Keys.onUpPressed: {
        if (!serverCountry.activeFocus) {
            serverCountry.forceActiveFocus();
        } else if (serverList.currentIndex > 0) {
            serverList.decrementCurrentIndex();
            if (serverList.currentItem.cityListVisible && !serverList.currentItem.isActive) {
                serverList.currentItem.activate();
            }
        }
    }
    Keys.onRightPressed: {
        if (!cityListVisible) {
            serverCountry.clicked();
        }
    }
    Keys.onLeftPressed: {
        if (cityListVisible) {
            serverCountry.clicked();
        }
    }

    function activate() {
        cityList.forceActiveFocus();
    }

    RowLayout {
        id: serverCountryRow

        spacing: 0
        height: 40
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

    ListView {
        id: cityList

        interactive: false
        opacity: 0
        enabled: opacity > 0
        model: cities
        spacing: Theme.listSpacing
        width: serverCountry.width - anchors.leftMargin
        height: contentItem.childrenRect.height
        anchors.top: serverCountryRow.bottom
        anchors.topMargin: 18
        anchors.left: serverCountry.left
        anchors.leftMargin: Theme.hSpacing + Theme.vSpacing + 6
        Behavior on opacity {
            NumberAnimation {
                easing.type: Easing.InSine
                duration: 200
            }
        }
        Accessible.role: Accessible.List
        //% "Cities"
        //: The title of the cities list.
        Accessible.name: qsTrId("cities")
        // Only allow focus within the current item in the list.
        activeFocusOnTab: serverCountry.ListView.isCurrentItem
        highlightFollowsCurrentItem: true
        Keys.onDownPressed: {
            if (cityList.currentIndex === cityList.count - 1) {
                event.accepted = false;
                return;
            }

            cityList.incrementCurrentIndex()
        }
        Keys.onUpPressed: {
            if (cityList.currentIndex === 0) {
                event.accepted = false;
                return;
            }

            cityList.decrementCurrentIndex()
        }

        highlight: VPNFocus {
            itemToFocus: cityList
            itemToAnchor: cityList.currentItem
        }

        delegate: VPNRadioDelegate {
            radioButtonLabelText: modelData
            accessibleName: modelData
            onClicked: {
                VPNController.changeServer(code, modelData);
                stackview.pop();
            }
            checked: code === VPNCurrentServer.countryCode && modelData === VPNCurrentServer.city
            isHoverable: cityList.enabled
        }

        footer: Rectangle {
            height: 16
            width: serverList.width
            color: "transparent"
        }

    }

}
