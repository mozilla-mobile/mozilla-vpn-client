/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    id: scrollingFrame

    ListModel {
        id: settingsMenuListModel
        ListElement {
            settingTitle: qsTr("Notifications")
            imageLeftSource: "../resources/settings/notifications.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNotifications.qml"
        }
        ListElement {
            settingTitle: qsTr("Network settings")
            imageLeftSource: "../resources/settings/networkSettings.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNetworkSettings.qml"
        }
        ListElement {
            settingTitle: qsTr("Language")
            imageLeftSource: "../resources/settings/language.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewLanguage.qml"
        }
        ListElement {
            settingTitle: qsTr("About us")
            imageLeftSource: "../resources/settings/aboutUs.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewAboutUs.qml"
        }
        ListElement {
            settingTitle: qsTr("Get help")
            imageLeftSource: "../resources/settings/getHelp.svg"
            imageRightSource: "../resources/chevron.svg"
            pushGetHelp: true
        }
        ListElement {
            settingTitle: qsTr("Give feedback")
            imageLeftSource: "../resources/settings/feedback.svg"
            imageRightSource: "../resources/externalLink.svg"
            openUrl: VPN.LinkFeedback
        }
    }

    contentHeight: 660
    boundsBehavior: Flickable.StopAtBounds

    VPNIconButton {
        id: iconButton
        onClicked: stackview.pop()

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2

        accessibleName: qsTr("Back")

        Image {
            id: backImage
            source: "../resources/back.svg"
            sourceSize.width: Theme.iconSize

            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }
    }

    Image {
        id: logo
        source: VPNUser.avatar
        anchors.horizontalCenter: parent.horizontalCenter
        height: 80
        smooth: true
        fillMode: Image.PreserveAspectFit
        layer.enabled: true
        anchors.top: parent.top
        anchors.topMargin: 32
        layer.effect: OpacityMask {
            maskSource: mask
        }
        Rectangle {
            id: mask
            anchors.fill: parent
            radius: 40
            visible: false
        }
    }

    VPNHeadline {
        id: logoTitle
        text: VPNUser.displayName ? VPNUser.displayName : qsTr("VPN User")
        anchors.top: logo.bottom
        anchors.topMargin: Theme.vSpacing
        height: 32
    }

    VPNSubtitle {
        id: logoSubtitle
        text: VPNUser.email
    }

    VPNButton {
        id: manageAccountButton
        text: qsTr("Manage account")
        anchors.top: logoSubtitle.bottom
        anchors.topMargin: Theme.vSpacing
        onClicked: VPN.openLink(VPN.LinkAccount)
    }

    VPNCheckBoxRow {
        id: startAtBootCheckBox
        labelText: qsTr("Launch VPN app on computer startup")
        subLabelText: ""
        isChecked: VPNSettings.startAtBoot
        isEnabled: true
        showDivider: true

        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.vSpacing

        onClicked: VPNSettings.startAtBoot = !VPNSettings.startAtBoot
    }

    Component {
        id: getHelpComponent
        VPNGetHelp {
            isSettingsView: true
        }
    }

    ListView {
        id: settingsList
        interactive: false // disable scrolling on list since the entire window is scrollable
        height: parent.height - manageAccountButton.height - logoSubtitle.height - logoTitle.height - startAtBootCheckBox.height
        width: parent.width
        anchors.top: startAtBootCheckBox.bottom
        anchors.topMargin: Theme.vSpacing
        spacing: Theme.listSpacing

        model: settingsMenuListModel
        delegate: VPNClickableRow {
            onClicked: {
                if (pushGetHelp) {
                    return settingsStackView.push(getHelpComponent)
                }
                if (pushView) {
                    return settingsStackView.push(pushView)
                }
                return VPN.openLink(openUrl)
            }

            VPNSettingsItem {
                setting: settingTitle
                imageLeftSrc: imageLeftSource
                imageRightSrc: imageRightSource
            }
        }
    }

    VPNFooterLink {
        id: signOutLink
        labelText: qsTr("Sign out")
        isBoldLink: true
        fontName: vpnFont.name
        onClicked: VPNController.logout()
        linkColor: Theme.redButton
    }

    ScrollBar.vertical: ScrollBar {}
}
