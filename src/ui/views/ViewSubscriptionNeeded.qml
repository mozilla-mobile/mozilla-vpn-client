/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme


VPNFlickable {
    id: vpnFlickable

    property var wasmView: false

    width: window.width
    flickContentHeight: productList

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp")
        onClicked: stackview.push(getHelpComponent)
    }

    ColumnLayout {
        anchors.top: headerLink.bottom
        width: vpnFlickable.width

        Repeater {
            id: productList
            model: VPNIAP
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true

            delegate: Rectangle {
                VPNTextBlock {
                   text: productIdentifier
                }
                VPNTextBlock {
                   text: productPrice
                }
                VPNTextBlock {
                   text: productType
                }
            }
        }
    }

    VPNSignOut {
        anchors.bottom: undefined
        anchors.bottomMargin: undefined
        anchors.horizontalCenter: undefined
        Layout.alignment: Qt.AlignHCenter
        onClicked: {
            VPNController.logout();
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: Theme.windowMargin * 2
        color: "transparent"
    }
}
