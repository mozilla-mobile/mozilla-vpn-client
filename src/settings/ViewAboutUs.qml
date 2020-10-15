/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Item {
    id: viewAboutUs

    ListModel {
        id: aboutUsListModel
        ListElement {
            //% "Terms of Service"
            linkTitle: qsTrId("tos")
            openUrl: VPN.LinkTermsOfService
        }
        ListElement {
            //% "Privacy Policy"
            linkTitle: qsTrId("privacyPolicy")
            openUrl: VPN.LinkPrivacyPolicy
        }
    }

    VPNMenu {
        id: menu
         //% "About us"
        title: qsTrId("aboutUs")
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
            text: qsTrId("productName")
        }

        VPNTextBlock {
            id: mozillaText
            text: qsTrId("productDescription")
            anchors.top: mozillaLabel.bottom
            anchors.topMargin: 8
        }

        VPNBoldLabel {
            id: releaseLabel
            //% "Release Version"
            text: qsTrId("releaseVersion")
            anchors.top: mozillaText.bottom
            anchors.topMargin: 16
        }

        VPNTextBlock {
            anchors.top: releaseLabel.bottom
            anchors.topMargin: 8
            text: VPN.versionString
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

    VPNList {
        id: settingList
        anchors.top: divider.bottom
        anchors.topMargin: 16
        anchors.bottomMargin: Theme.vSpacing
        height: parent.height - menu.height - aboutUsCopy.height - divider.height
        width: viewAboutUs.width
        spacing: Theme.listSpacing
        model: aboutUsListModel
        listName: menu.title

        delegate: VPNExternalLinkListItem {
            title: linkTitle
            accessibleName: linkTitle
            onClicked: VPN.openLink(openUrl)
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }
    }
}
