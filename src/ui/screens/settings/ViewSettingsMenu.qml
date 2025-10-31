/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    id: vpnFlickable
    objectName: "settingsView"
    _accessibleName: MZI18n.NavBarSettingsTab

    _menuTitle: MZI18n.NavBarSettingsTab

    readonly property string telemetryScreenId : "settings"

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        ColumnLayout {
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin /2
            Layout.rightMargin: MZTheme.theme.windowMargin /2
            Layout.alignment: Qt.AlignHCenter

            MZSettingsItem {
                objectName: "settingsUserProfile"
                settingTitle: MZI18n.SubscriptionManagementSectionTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconAvatar")
                imageRightSrc: MZAssetLookup.getImageSource("ExternalLink")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                   MZUrlOpener.openUrlLabel("account");
                }
            }

            MZSettingsItem {
                objectName: "privacySettings"
                settingTitle: MZI18n.SettingsPrivacySettings
                imageLeftSrc: MZAssetLookup.getImageSource("IconPrivacyMask")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/privacy/ViewPrivacy.qml")
                }
            }

            MZSettingsItem {
                objectName: "appExclusionSettings"
                settingTitle: MZI18n.SettingsAppExclusionTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconPermissions")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/appPermissions/ViewAppPermissions.qml")
                }
                visible: MZFeatureList.get("splitTunnel").isSupported
            }

            MZSettingsItem {
                objectName: "firefoxExtensionInfo"
                settingTitle: MZI18n.SettingsFirefoxExtensionInfo
                imageLeftSrc: MZAssetLookup.getImageSource("IconPuzzlePiece")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewFirefoxExtensionInfo.qml")
                }
                visible: Qt.platform.os === "windows"
            }

            MZSettingsItem {
                objectName: "settingsDevice"

                //% "Devices"
                settingTitle: MZI18n.DevicesSectionTitle
                imageLeftSrc: MZAssetLookup.getImageSource("IconDevices")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/devices/ViewDevices.qml")
                }
            }

            MZSettingsItem {
                id: preferencesSetting
                objectName: "settingsPreferences"
                settingTitle: MZI18n.SettingsPreferencesSettings
                imageLeftSrc: MZAssetLookup.getImageSource("IconWrench")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewPreferences.qml", {
                                                    _startAtBootTitle: Qt.binding(() => MZI18n.SettingsStartAtBootTitle),
                                                    _languageTitle:  Qt.binding(() => qsTrId("vpn.settings.language")),
                                                    _notificationsTitle:  Qt.binding(() => qsTrId("vpn.settings.notifications")),
                                                    _menuTitle: Qt.binding(() => preferencesSetting.settingTitle)
                                                  })
                }
            }

            MZSettingsItem {
                objectName: "settingsGetHelp"
                settingTitle: MZI18n.GetHelpLinkText
                imageLeftSrc: MZAssetLookup.getImageSource("MenuIconQuestion")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    MZNavigator.requestScreen(VPN.ScreenGetHelp);
                }
            }

            MZSettingsItem {
                objectName: "settingsAboutUs"
                settingTitle: MZI18n.AboutUsTitle
                imageLeftSrc: MZAssetLookup.getImageSource("InfoIcon")
                imageRightSrc: MZAssetLookup.getImageSource("Chevron")
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewAboutUs.qml")
                }
            }

            MZSignOut {
                id: signOutLink
                objectName: "accountLogout"
                anchors {
                    horizontalCenter: undefined
                    bottom: undefined
                    bottomMargin: undefined
                }
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                Layout.topMargin: MZTheme.theme.vSpacingSmall - parent.spacing
            }
        }
    }
}
