/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15

VPNFlickable {
    id: vpnFlickable
    objectName: "settingsView"
    flickContentHeight: settingsList.y + settingsList.height + signOutLink.height + signOutLink.anchors.bottomMargin
    hideScollBarOnStackTransition: true

    VPNIconButton {
        id: iconButton
        objectName: "settingsCloseButton"
        onClicked: stackview.pop(StackView.Immediate)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        accessibleName: qsTrId("vpn.connectionInfo.close")

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
        logo: VPNUser.avatar
        //% "VPN User"
        readonly property var textVpnUser: qsTrId("vpn.settings.user")
        logoTitle: VPNUser.displayName ? VPNUser.displayName : textVpnUser
        logoSubtitle: VPNUser.email
        anchors.top: parent.top
        anchors.topMargin: (Math.max(window.safeContentHeight * .08, Theme.windowMargin * 2))
        maskImage: true
        isSettingsView: true
    }

    VPNButton {
        id: manageAccountButton
        objectName: "manageAccountButton"
        text: qsTrId("vpn.main.manageAccount")
        anchors.top: vpnPanel.bottom
        anchors.topMargin: Theme.vSpacing
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
            Sample.manageAccountClicked.record();
            VPN.openLink(VPN.LinkAccount)
        }
    }

    Component {
        id: aboutUsComponent

        VPNAboutUs {
            isSettingsView: true
            isMainView: false
        }
    }

    ColumnLayout {
        id: settingsList

        spacing: Theme.listSpacing
        y: Theme.vSpacing + manageAccountButton.y + manageAccountButton.height
        width: parent.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        VPNSettingsItem {
            objectName: "settingsWhatsNew"
            settingTitle: VPNl18n.tr(VPNl18n.WhatsNewReleaseNotesTourPageHeader)
            imageLeftSrc: "../resources/gift-dark.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewWhatsNew.qml")
            showIndicator: unseenFeaturesModel.rowCount() > 0
        }

        VPNSettingsItem {
            objectName: "settingsNetworking"
            settingTitle: qsTrId("vpn.settings.networking")
            imageLeftSrc: "../resources/settings/networkSettings.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewNetworkSettings.qml", {
                                                  //% "App permissions"
                                                  _appPermissionsTitle: qsTrId("vpn.settings.appPermissions2")
                                              } )
        }
        VPNSettingsItem {
            property string prefs: qsTrId("systray.preferences")
            property string notificationsTitle: qsTrId("vpn.settings.notifications")
            property string languageTitle: qsTrId("vpn.settings.language")
            //% "Launch VPN app on startup
            property string startAtBootTitle: qsTrId("vpn.settings.runOnBoot2")

            id: preferencesSetting
            objectName: "settingsPreferences"
            settingTitle: prefs.replace("…", "").replace("...", "")
            imageLeftSrc: "../resources/settings/preferences.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push("../settings/ViewPrivacySecurity.qml", {
                                                _startAtBootTitle: startAtBootTitle,
                                                _languageTitle: qsTrId("vpn.settings.language"),
                                                _notificationsTitle: qsTrId("vpn.settings.notifications"),
                                                _menuTitle: preferencesSetting.settingTitle
                                              })
        }

        VPNSettingsItem {
            //% "Give feedback"
            property string giveFeedbackTitle: qsTrId("vpn.settings.giveFeedback")
            objectName: "settingsGetHelp"
            settingTitle: qsTrId("vpn.main.getHelp2")
            imageLeftSrc: "../resources/settings/questionMark.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: {
                Sample.getHelpClickedViewSettings.record();
                settingsStackView.push("../views/ViewGetHelp.qml", {
                                           isSettingsView: true
                                       })
            }
        }

        VPNSettingsItem {
            objectName: "settingsAboutUs"
            settingTitle: qsTrId("vpn.settings.aboutUs")
            imageLeftSrc: "../resources/settings/aboutUs.svg"
            imageRightSrc: "../resources/chevron.svg"
            onClicked: settingsStackView.push(aboutUsComponent)
        }

        Rectangle {
            Layout.preferredHeight: fullscreenRequired? Theme.rowHeight * 1.5 : Theme.rowHeight
            Layout.fillWidth: true
            color: "transparent"
        }
    }

    VPNSignOut {
        id: signOutLink

        objectName: "settingsLogout"
        onClicked: VPNController.logout()
    }
}
