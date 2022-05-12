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
    property string _menuTitle: qsTrId("vpn.main.settings")

    id: vpnFlickable
    objectName: "settingsView"
    flickContentHeight: settingsList.implicitHeight

    Component {
        id: aboutUsComponent

        VPNAboutUs {
            licenseURL: "qrc:/ui/views/ViewLicenses.qml"
        }
    }

    ColumnLayout {
        id: settingsList

        spacing: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin
        height: Math.max(vpnFlickable.height, settingsList.implicitHeight)

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }

        ColumnLayout {
            spacing: 0

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
            }

            VPNUserProfile {
                _iconButtonImageSource: "qrc:/nebula/resources/chevron.svg"
                _iconButtonOnClicked: () => {
                    Sample.manageAccountClicked.record();
                    VPN.openLink(VPN.LinkAccount)
                }
                Layout.leftMargin: VPNTheme.theme.windowMargin / 2
            }

            VPNVerticalSpacer {
                Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
            }

            Rectangle {
                id: divider

                Layout.preferredHeight: 1
                Layout.fillWidth: true
                Layout.leftMargin: VPNTheme.theme.windowMargin / 2
                Layout.rightMargin: VPNTheme.theme.windowMargin / 2
                color: VPNTheme.colors.grey10
            }
        }


        // TODO: Move to subscription management
        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width

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
                objectName: "settingsTipsAndTricks"
                settingTitle: VPNl18n.TipsAndTricksSettingsEntryLabel
                imageLeftSrc: "qrc:/nebula/resources/sparkles.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStackView.push("qrc:/ui/settings/ViewTipsAndTricks.qml")
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
                    getHelpViewNeeded();
                }
            }

            VPNSettingsItem {
                objectName: "settingsAboutUs"
                settingTitle: qsTrId("vpn.settings.aboutUs")
                imageLeftSrc: "qrc:/ui/resources/settings/aboutUs.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                onClicked: settingsStackView.push(aboutUsComponent)
            }

            VPNLinkButton {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: VPNTheme.theme.vSpacing

                fontName: VPNTheme.theme.fontBoldFamily
                labelText: VPNl18n.DeleteAccountButtonLabel + " (WIP)"
                linkColor: VPNTheme.theme.redButton
                visible: VPNFeatureList.get("accountDeletion").isSupported
                onClicked: {
                    settingsStackView.push("qrc:/ui/deleteAccount/ViewDeleteAccount.qml");
                }
            }

            VPNVerticalSpacer {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            VPNSignOut {
                id: signOutLink

                objectName: "settingsLogout"
                anchors {
                    horizontalCenter: undefined
                    bottom: undefined
                    bottomMargin: undefined
                }
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            }

            VPNVerticalSpacer {
                Layout.fillWidth: true
                Layout.minimumHeight: VPNTheme.theme.rowHeight
            }
        }
    }
}

