/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: viewAboutUs
    property alias isSettingsView: menu.isSettingsView

    ListModel {
        id: aboutUsListModel

        ListElement {
            linkId: "tos"

            //% "Terms of Service"
            linkTitle: qsTrId("vpn.aboutUs.tos")
            openUrl: VPN.LinkTermsOfService
        }

        ListElement {
            linkId: "privacy"

            //% "Privacy Notice"
            linkTitle: qsTrId("vpn.aboutUs.privacyNotice")
            openUrl: VPN.LinkPrivacyNotice
        }
        ListElement {
            linkId: "license"

            //% "License"
            linkTitle: qsTrId("vpn.aboutUs.license")
            openUrl: VPN.LinkLicense
        }

    }

    VPNMenu {
        id: menu
        objectName: "aboutUsBackButton"

        //% "About us"
        title: qsTrId("vpn.settings.aboutUs")
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
            text: qsTrId("vpn.main.productName")
        }

        VPNTextBlock {
            id: mozillaText

            text: qsTrId("vpn.main.productDescription")
            anchors.top: mozillaLabel.bottom
            anchors.topMargin: 8
            width: aboutUsCopy.width
        }

        VPNBoldLabel {
            id: releaseLabel

            //% "Release Version"
            //: Refers to the installed version. For example: "Release Version: 1.23"
            text: qsTrId("vpn.aboutUs.releaseVersion")
            anchors.top: mozillaText.bottom
            anchors.topMargin: 16
        }

        TextEdit {
            anchors.top: releaseLabel.bottom
            anchors.topMargin: 8
            text: VPN.buildNumber === "" ? VPN.versionString : (VPN.versionString + " (" + VPN.buildNumber + ")")
            readOnly: true
            wrapMode: Text.WordWrap
            selectByMouse: true

            color: Theme.fontColor
            font.family: Theme.fontInterFamily
            font.pixelSize: Theme.fontSizeSmall
            width: Theme.maxTextWidth

            Accessible.role: Accessible.StaticText
            Accessible.name: text
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
        objectName: "aboutUsList"

        anchors.top: divider.bottom
        anchors.topMargin: 16
        anchors.bottomMargin: Theme.vSpacing
        height: parent.height - menu.height - aboutUsCopy.height - divider.height
        width: viewAboutUs.width
        spacing: Theme.listSpacing
        model: aboutUsListModel
        listName: menu.title

        delegate: VPNExternalLinkListItem {
            objectName: "aboutUsList-" + linkId
            title: linkTitle
            accessibleName: linkTitle
            onClicked: VPN.openLink(openUrl)
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
