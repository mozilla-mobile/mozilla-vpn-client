import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: viewAboutUs

    ListModel {
        id: aboutUsListModel
        ListElement {
            linkTitle: qsTr("Terms of Service")
            openUrl: VPN.LinkTermsOfService
        }
        ListElement {
            linkTitle: qsTr("Privacy Policy")
            openUrl: VPN.LinkPrivacyPolicy
        }
    }

    VPNMenu {
        id: menu
        title: qsTr("About us")
        isSettingsView: true
    }

    Rectangle {
        id: aboutUsCopy
        anchors.top: menu.bottom
        anchors.left: viewAboutUs.left
        anchors.topMargin: Theme.vSpacing
        anchors.leftMargin: Theme.windowMargin
        anchors.rightMargin: Theme.windowMargin
        height: childrenRect.height
        width: viewAboutUs.width - (Theme.windowMargin * 2)
        color: "transparent"

        VPNBoldLabel {
            id: mozillaLabel
            width: aboutUsCopy.width
            text: qsTr("Mozilla VPN")
        }

        VPNTextBlock {
            id: mozillaText
            text: qsTr("A fast, secure and easy to use VPN. Built by the makers of Firefox.")
            anchors.top: mozillaLabel.bottom
            anchors.topMargin: 8
        }

        VPNBoldLabel {
            id: releaseLabel
            text: qsTr("Release Version")
            anchors.top: mozillaText.bottom
            anchors.topMargin: 16
        }

        VPNTextBlock {
            anchors.top: releaseLabel.bottom
            anchors.topMargin: 8
            text: VPN.versionString // TODO - Needs a "Notify" in case this changes
        }
    }

    Rectangle {
        id: divider
        height: 1
        width: viewAboutUs.width
        anchors.top: aboutUsCopy.bottom
        anchors.left: viewAboutUs.left
        anchors.right: viewAboutUs.right
        anchors.topMargin: 12
        anchors.leftMargin: Theme.windowMargin
        anchors.rightMargin: Theme.windowMargin
        color: "#0C0C0D0A"
    }

    ListView {
        id: settingList
        anchors.top: divider.bottom
        anchors.topMargin: 16
        anchors.bottomMargin: Theme.vSpacing
        height: contentItem.childrenRect.height
        width: viewAboutUs.width
        spacing: Theme.listSpacing
        interactive: false
        model: aboutUsListModel

        delegate: VPNExternalLinkListItem {
            title: linkTitle
            onClicked: VPN.openLink(openUrl)
        }
    }
}
