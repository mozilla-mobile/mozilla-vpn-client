/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

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
        anchors.topMargin: VPNTheme.theme.windowMargin / 2
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        accessibleName: qsTrId("vpn.connectionInfo.close")

        Image {
            id: backImage

            source: "qrc:/nebula/resources/close-dark.svg"
            sourceSize.width: VPNTheme.theme.iconSize
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
        anchors.topMargin: (Math.max(window.safeContentHeight * .08, VPNTheme.theme.windowMargin * 2))
        isSettingsView: true
    }

    VPNButton {
        id: manageAccountButton
        objectName: "manageAccountButton"
        text: qsTrId("vpn.main.manageAccount")
        anchors.top: vpnPanel.bottom
        anchors.topMargin: VPNTheme.theme.vSpacing
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
            licenseURL: "qrc:/ui/views/ViewLicenses.qml"
        }
    }

    ColumnLayout {
        id: settingsList

        spacing: VPNTheme.theme.listSpacing
        y: VPNTheme.theme.vSpacing + manageAccountButton.y + manageAccountButton.height
        width: parent.width - VPNTheme.theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter

        VPNSettingsItem {
            objectName: "settingsWhatsNew"
            settingTitle: VPNl18n.WhatsNewReleaseNotesTourPageHeader
            imageLeftSrc: "qrc:/nebula/resources/gift-dark.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            onClicked: settingsStackView.push("qrc:/ui/settings/ViewWhatsNew.qml")
            showIndicator: VPNWhatsNewModel.hasUnseenFeature
            visible: VPNWhatsNewModel.rowCount() > 0
        }

        VPNSettingsItem {
            objectName: "settingsNetworking"
            settingTitle: qsTrId("vpn.settings.networking")
            imageLeftSrc: "qrc:/ui/resources/settings/networkSettings.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            onClicked: settingsStackView.push("qrc:/ui/settings/ViewNetworkSettings.qml", {
                                                  //% "App permissions"
                                                  _appPermissionsTitle: Qt.binding(() => qsTrId("vpn.settings.appPermissions2"))
                                              })
        }

        VPNSettingsItem {
            id: preferencesSetting
            objectName: "settingsPreferences"
            settingTitle: VPNl18n.SettingsSystemPreferences
            imageLeftSrc: "qrc:/ui/resources/settings/preferences.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            onClicked: settingsStackView.push("qrc:/ui/settings/ViewPrivacySecurity.qml", {
                                                _startAtBootTitle: Qt.binding(() => VPNl18n.SettingsStartAtBootTitle),
                                                _languageTitle:  Qt.binding(() => qsTrId("vpn.settings.language")),
                                                _notificationsTitle:  Qt.binding(() => qsTrId("vpn.settings.notifications")),
                                                _menuTitle: Qt.binding(() => preferencesSetting.settingTitle)
                                              })
        }

        VPNSettingsItem {
            //% "Give feedback"
            property string giveFeedbackTitle: qsTrId("vpn.settings.giveFeedback")
            objectName: "settingsGetHelp"
            settingTitle: qsTrId("vpn.main.getHelp2")
            imageLeftSrc: "qrc:/ui/resources/settings/questionMark.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            onClicked: {
                Sample.getHelpClickedViewSettings.record();
                settingsStackView.push("qrc:/ui/views/ViewGetHelp.qml", {isSettingsView: true})
            }
        }

        VPNSettingsItem {
            objectName: "settingsAboutUs"
            settingTitle: qsTrId("vpn.settings.aboutUs")
            imageLeftSrc: "qrc:/ui/resources/settings/aboutUs.svg"
            imageRightSrc: "qrc:/nebula/resources/chevron.svg"
            onClicked: settingsStackView.push(aboutUsComponent)
        }

        Rectangle {
            Layout.preferredHeight: fullscreenRequired? VPNTheme.theme.rowHeight * 1.5 : VPNTheme.theme.rowHeight
            Layout.fillWidth: true
            color: VPNTheme.theme.transparent
        }
    }

    VPNSignOut {
        id: signOutLink

        objectName: "settingsLogout"
    }
}
