/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: viewInitialize

    VPNHeaderLink {
        id: headerLink

        labelText: qsTrId("vpn.main.getHelp")
        onClicked: stackview.push(getHelpComponent)
    }

    Component {
        id: getHelpComponent

        VPNGetHelp {
            isSettingsView: false
        }

    }

    VPNPanel {
        logo: "../resources/logo.svg"
        logoTitle: qsTrId("vpn.main.productName")
        //% "A fast, secure and easy to use VPN. Built by the makers of Firefox."
        logoSubtitle: qsTrId("vpn.main.productDescription")
    }

    VPNButton {
        id: getStarted

        anchors.bottom: learnMore.top
        anchors.bottomMargin: 24
        width: 282
        //% "Get started"
        text: qsTrId("vpn.main.getStarted")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: {
            fadeOut.start();
            VPN.authenticate();
        }
    }

    VPNFooterLink {
        id: learnMore

        //% "Learn more"
        labelText: qsTrId("vpn.main.learnMore")
        onClicked: stackview.push("ViewOnboarding.qml")
    }

    PropertyAnimation on opacity {
        id: fadeOut

        target: viewInitialize
        from: 1
        to: 0
        duration: 200
    }

}
