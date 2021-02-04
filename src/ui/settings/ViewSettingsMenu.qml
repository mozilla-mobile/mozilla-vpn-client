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

    width: window.width
    flickContentHeight: settingsList.y + settingsList.height + signOutLink.height + signOutLink.anchors.bottomMargin
    hideScollBarOnStackTransition: true

    VPNIconButton {
        id: iconButton

        onClicked: stackview.pop(StackView.Immediate)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        accessibleName: qsTrId("vpn.main.back")

        Image {
            id: backImage

            source: "../resources/close-dark.svg"
            sourceSize.width: Theme.iconSize
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }

    }

    VPNPanel {
        id: vpnPanel
        logoSize: 80
        logo:  VPNUser.avatar
        //% "VPN User"
        readonly property var textVpnUser: qsTrId("vpn.settings.user")
        logoTitle: VPNUser.displayName ? VPNUser.displayName : textVpnUser
        logoSubtitle: VPNUser.email
        y: (Math.max(window.safeContentHeight * .08, Theme.windowMargin * 2))
        maskImage: true
        imageIsVector: false
    }

    VPNButton {
        id: manageAccountButton

        text: qsTrId("vpn.main.manageAccount")
        anchors.top: vpnPanel.bottom
        anchors.topMargin: Theme.vSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: VPN.openLink(VPN.LinkAccount)
    }

    VPNCheckBoxRow {
        id: startAtBootCheckBox

        //% "Launch VPN app on Startup"
        labelText: qsTrId("vpn.settings.runOnBoot")
        subLabelText: ""
        isChecked: VPNSettings.startAtBoot
        isEnabled: true
        showDivider: true
        anchors.top: manageAccountButton.bottom
        anchors.topMargin: Theme.hSpacing * 1.5
        anchors.rightMargin: Theme.hSpacing
        width: vpnFlickable.width - Theme.hSpacing
        onClicked: VPNSettings.startAtBoot = !VPNSettings.startAtBoot

        visible: VPN.startOnBootSupported
    }

    Component {
        id: getHelpComponent

        VPNGetHelp {
            isSettingsView: true
        }

    }

    Component {
        id: aboutUsComponent

        VPNAboutUs {
            isSettingsView: true
        }
    }


    ColumnLayout {
        id: settingsList

        spacing: Theme.listSpacing
        y: Theme.vSpacing + (VPN.startOnBootSupported ? startAtBootCheckBox.y + startAtBootCheckBox.height : manageAccountButton.y + manageAccountButton.height)
        width: parent.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter

        VPNSettingsItem {
            settingTitle: qsTrId("vpn.settings.networking")
            imageLeftSrc: "../resources/settings/networkSettings.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewNetworkSettings.qml")
        }
        VPNSettingsItem {
            settingTitle: qsTrId("vpn.settings.language")
            imageLeftSrc: "../resources/settings/language.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewLanguage.qml")
            visible: VPNLocalizer.hasLanguages
        }
        VPNSettingsItem {
            //% "App Permissions"
            settingTitle: qsTrId("vpn.settings.appPermissions")
            imageLeftSrc: "../resources/settings/apps.svg"
            imageRightSrc: "../resources/chevron.svg"
            visible: VPN.protectSelectedAppsSupported
            onClicked: settingsStackView.push("../settings/ViewAppPermissions.qml")
        }
        VPNSettingsItem {
            settingTitle: qsTrId("vpn.settings.aboutUs")
            imageLeftSrc: "../resources/settings/aboutUs.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push(aboutUsComponent)
        }
        VPNSettingsItem {
            //% "Give feedback"
            settingTitle: qsTrId("vpn.settings.giveFeedback")
            imageLeftSrc: "../resources/settings/feedback.svg"
            imageRightSrc: "../resources/externalLink.svg"
            onClicked: VPN.openLink(VPN.LinkFeedback)
        }
        VPNSettingsItem {
            settingTitle: qsTrId("vpn.main.getHelp")
            imageLeftSrc: "../resources/settings/getHelp.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push(getHelpComponent)
        }

        /* TODO - disable captive portal and its notifications
        VPNSettingsItem {
            settingTitle: qsTrId("vpn.settings.notifications")
            imageLeftSrc: "../resources/settings/notifications.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewNotifications.qml")
        }
        */

        Rectangle {
            Layout.preferredHeight: fullscreenRequired? Theme.rowHeight * 1.5 : Theme.rowHeight
            Layout.fillWidth: true
            color: "transparent"
        }
    }

    VPNSignOut {
        id: signOutLink

        onClicked: VPNController.logout()
    }

}
