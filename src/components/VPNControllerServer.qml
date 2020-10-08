/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

VPNClickableRow {
    id: servers
    //% "Select location"
    //: Select the Location of the VPN server
    property var titleText: qsTrId("selectLocation")
    //% "current location - %1"
    //: Accessibility description for current location of the VPN server
    property var descriptionText: qsTrId("currentLocation").arg(VPNCurrentServer.city)
    accessibleName: `${titleText}: ${descriptionText}`
    rowShouldBeDisabled: VPNController.state === VPNController.StateDeviceLimit

    activeFocusOnTab: true
    VPNFocus {
        itemToFocus: servers
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 0

        VPNIconAndLabel {
            icon: "../resources/connection.svg"
            title: titleText
            Accessible.ignored: true
        }

        Item {
            Layout.fillWidth: true
        }

        Image {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            Layout.rightMargin: 8
            fillMode: Image.PreserveAspectFit
            source: "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase(
                        ) + ".png"
        }

        VPNLightLabel {
            text: VPNCurrentServer.city
            Layout.rightMargin: 8
            Accessible.ignored: true
        }

        VPNChevron { }
    }
}
