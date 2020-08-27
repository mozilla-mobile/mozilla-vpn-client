import QtQuick 2.0
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    VPNHeaderLink {
        text: qsTr("Get Help")
        onClicked: stackview.push("ViewGetHelp.qml")
    }

    VPNPanel {
        logo: "../resources/logo.svg"
        logoTitle: qsTr("Mozilla VPN")
        logoSubtitle: qsTr("A fast, secure and easy to use VPN.")
    }

    VPNButton {
        id: getStarted
        anchors.bottom: learnMore.top
        anchors.bottomMargin: 24
        width: 282
        text: qsTr("Get started")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 5
        onClicked: VPN.authenticate()
    }

    VPNFooterLink {
        id: learnMore
        text: qsTr("Learn more")
        onClicked: stackview.push("ViewOnboarding.qml")
    }
}
