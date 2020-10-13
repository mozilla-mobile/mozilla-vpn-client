/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    id: scrollingFrame

    ListModel {
        id: settingsMenuListModel
        ListElement {
            settingTitle: qsTrId("notifications")
            imageLeftSource: "../resources/settings/notifications.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNotifications.qml"
        }
        ListElement {
            settingTitle: qsTrId("networkSettings")
            imageLeftSource: "../resources/settings/networkSettings.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewNetworkSettings.qml"
        }
        ListElement {
            settingTitle: qsTrId("language")
            imageLeftSource: "../resources/settings/language.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewLanguage.qml"
        }
        ListElement {
            settingTitle: qsTrId("aboutUs")
            imageLeftSource: "../resources/settings/aboutUs.svg"
            imageRightSource: "../resources/chevron.svg"
            pushView: "../settings/ViewAboutUs.qml"
        }
        ListElement {
            settingTitle: qsTrId("getHelp")
            imageLeftSource: "../resources/settings/getHelp.svg"
            imageRightSource: "../resources/chevron.svg"
            pushGetHelp: true
        }
        ListElement {
            //% "Give feedback"
            settingTitle: qsTrId("giveFeedback")
            imageLeftSource: "../resources/settings/feedback.svg"
            imageRightSource: "../resources/externalLink.svg"
            openUrl: VPN.LinkFeedback
        }
    }

    contentHeight: 720
    height: parent.height

    boundsBehavior: Flickable.StopAtBounds

    VPNIconButton {
        id: iconButton
        onClicked: stackview.pop()

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2

        accessibleName: qsTrId("back")

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
 
        text: VPNUser.displayName ? 
                    VPNUser.displayName : 
                    //% "VPN User"
                    qsTrId("vpnUser")
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
        //: "Manage account"
        text: qsTrId("manageAccount")
        anchors.top: logoSubtitle.bottom
        anchors.topMargin: Theme.vSpacing
        onClicked: VPN.openLink(VPN.LinkAccount)
    }

    VPNCheckBoxRow {
        id: startAtBootCheckBox
        //: The back of the object, not the front
        //% "Launch VPN app on Startup"
        labelText: qsTrId("runOnBoot")
        subLabelText: ""
        isChecked: VPNSettings.startAtBoot
        isEnabled: true
        showDivider: true

        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.hSpacing * 1.5
        anchors.rightMargin: Theme.hSpacing
        width: parent.width - Theme.hSpacing
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
        height: settingsList.count * (40 + (Theme.listSpacing * 2))
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
        labelText: qsTrId("signOut")
        isBoldLink: true
        fontName: Theme.fontFamily
        onClicked: VPNController.logout()
        linkColor: Theme.redButton
    }

    ScrollBar.vertical: ScrollBar {}
}
