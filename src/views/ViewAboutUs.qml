import QtQuick 2.0
import QtQuick.Controls 2.5

import Mozilla.VPN 1.0

import "../components"

Item {
    VPNMenu {
        id: menu
        title: qsTr("About Us")
    }

    VPNBoldLabel {
        id: mozillaLabel
        text: qsTr("Mozilla VPN")
        anchors.top: menu.bottom
        anchors.left: parent.left
        anchors.margins: 16
    }

    VPNLightLabel {
        id: mozillaText
        text: qsTr("A fast, secure and easy to use VPN. Built by the makers of Firefox.")
        width: parent.width
        anchors.top: mozillaLabel.bottom
        anchors.left: parent.left
        anchors.margins: 16
    }

    VPNBoldLabel {
        id: releaseLabel
        text: qsTr("Release Version")
        anchors.top: mozillaText.bottom
        anchors.left: parent.left
        anchors.margins: 16
    }

    VPNLightLabel {
        id: releaseText
        text: VPN.versionString
        width: parent.width
        anchors.top: releaseLabel.bottom
        anchors.left: parent.left
        anchors.margins: 16
    }

    // TODO: term of service link -> VPN.LinkTermsOfService
    // TODO: privacy policy link -> VPN.LinkPrivacyPolicy
}
