/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

Item {
    id: viewAboutUs
    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
    //% "About us"
    property string _menuTitle: qsTrId("vpn.settings.aboutUs")

    ListModel {
        id: aboutUsListModel

        ListElement {
            linkId: "tos"

            //% "Terms of service"
            linkTitle: qsTrId("vpn.aboutUs.tos2")
            openUrl: VPN.LinkTermsOfService
        }

        ListElement {
            linkId: "privacy"

            //% "Privacy notice"
            linkTitle: qsTrId("vpn.aboutUs.privacyNotice2")
            openUrl: VPN.LinkPrivacyNotice
        }

        ListElement {
            linkId: "license"

            linkTitleId: "AboutUsLicenses"
            //TODO: Forward ref to UI
            openView: "qrc:/ui/views/ViewLicenses.qml"
        }
    }

    VPNMenu {
        id: menu
        objectName: "aboutUsBackButton"
        title: qsTrId("vpn.settings.aboutUs")
        visible: !isSettingsView
    }

    Rectangle {
        id: aboutUsCopy

        anchors.top: menu.visible ? menu.bottom : parent.top
        anchors.topMargin: menu.visible ? 0 : Theme.menuHeight + Theme.windowMargin
        anchors.left: viewAboutUs.left
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

            //% "Release version"
            //: Refers to the installed version. For example: "Release Version: 1.23"
            text: qsTrId("vpn.aboutUs.releaseVersion2")
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
        height: parent.height
        width: viewAboutUs.width
        spacing: Theme.listSpacing
        model: aboutUsListModel
        listName: _menuTitle

        delegate: VPNExternalLinkListItem {
            objectName: "aboutUsList-" + linkId
            title: linkTitleId ? VPNl18n[linkTitleId] : linkTitle
            accessibleName: title
            onClicked: {
                if (openUrl) {
                    VPN.openLink(openUrl)
                }
                if (openView) {
                    if (isSettingsView) {
                        settingsStackView.push(openView, { isSettingsView, isMainView })
                    } else if (isMainView) {
                        mainStackView.push(openView, { isSettingsView, isMainView })
                    } else {
                        stackview.push(openView, { isSettingsView, isMainView })
                    }
                }
            }
            iconSource: openUrl ? "qrc:/nebula/resources/externalLink.svg" : "qrc:/nebula/resources/chevron.svg"
        }

        ScrollBar.vertical: ScrollBar {
            Accessible.ignored: true
        }

    }

}
