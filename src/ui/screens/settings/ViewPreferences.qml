/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: vpnFlickable
    property string _startAtBootTitle: ""
    property string _notificationsTitle: ""
    property string _languageTitle: ""
    readonly property string telemetryScreenId : "app_preferences"

    property bool vpnIsOff: VPNController.state === VPNController.StateOff
    property bool isIOS: Qt.platform.os === "ios"

    objectName: "settingsPreferencesView"

    Component.onCompleted: Glean.impression.appPreferencesScreen.record({screen:telemetryScreenId})

    _viewContentData: ColumnLayout {
        Layout.preferredWidth: parent.width
        Layout.alignment: Qt.AlignHCenter

        spacing: MZTheme.theme.windowMargin

        // MZTextBlock {
        //     id: subLabel
        //     Layout.fillWidth: true

        //     text: "HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILINE>HELLO WORLD WE AR EMULTILIN"
        //     font.pixelSize: MZTheme.theme.fontSizeSmall
        //     visible: !!text.length
        //     wrapMode: Text.Wrap
        // }

        MZExternalLinkListItem {
            objectName: "androidStartAtBootLink"

            // height: 80
            // canGrowVertical: false
            accessibleName: _startAtBootTitle
            title: _startAtBootTitle
            subLabelText: MZI18n.SettingsStartAtBootSubtitle
            iconSource: "qrc:/nebula/resources/externalLink.svg"
            visible: Qt.platform.os === "android"
            backgroundColor: MZTheme.theme.clickableRowBlue
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.toggleRowDividerSpacing
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin
            onClicked: MZUrlOpener.openUrl("https://support.mozilla.org/kb/how-enable-always-vpn-android")
        }

        MZToggleRow {
            objectName: "startAtBootToogle"

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin

            labelText: _startAtBootTitle
            subLabelText: MZI18n.SettingsStartAtBootSubtitle
            checked: MZSettings.startAtBoot
            visible: MZFeatureList.get("startOnBoot").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: {
                MZSettings.startAtBoot = !MZSettings.startAtBoot
                if (MZSettings.startAtBoot)
                {
                    Glean.interaction.vpnOnStartupEnabled.record({screen:telemetryScreenId})
                }
                else
                {
                    Glean.interaction.vpnOnStartupDisabled.record({screen:telemetryScreenId})
                }
            }
        }

        MZToggleRow {
            objectName: "dataCollectionToggle"

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin
            showDivider: isIOS

            labelText: MZI18n.TelemetryPolicyViewShareTechnicalData
            subLabelText: MZI18n.SettingsShareTechnicalDataDescription
            checked: MZSettings.gleanEnabled
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.gleanEnabled = !MZSettings.gleanEnabled
        }

        MZToggleRow {
            id: localNetwork
            objectName: "settingLocalNetworkAccess"
            visible: isIOS

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin
            showDivider: false

            labelText: MZI18n.LocalNetworkAccessLabelTitle
            subLabelText: vpnIsOff ? MZI18n.LocalNetworkAccessSubLabel : MZI18n.LocalNetworkAccessDisabledSubLabel
            checked: MZSettings.localNetworkAccess
            enabled: isIOS && vpnIsOff
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: {
                if (VPNController.StateOff) {
                    MZSettings.localNetworkAccess = !MZSettings.localNetworkAccess
                }
            }
        }

        ColumnLayout {
            spacing: MZTheme.theme.windowMargin / 2
            Layout.alignment: Qt.AlignHCenter
            Layout.leftMargin: MZTheme.theme.windowMargin / 2
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            MZSettingsItem {
                objectName: "settingsNotifications"

                settingTitle: _notificationsTitle
                imageLeftSrc: "qrc:/ui/resources/settings/notifications.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.notificationsSelected.record({screen:telemetryScreenId})
                    if(Qt.platform.os === "android"){
                        VPNAndroidUtils.openNotificationSettings();
                        return;
                    }
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewNotifications.qml")
                }
                visible: MZFeatureList.get("captivePortal").isSupported || MZFeatureList.get("unsecuredNetworkNotification").isSupported || MZFeatureList.get("notificationControl").isSupported
            }

            MZSettingsItem {
                objectName: "settingsLanguages"

                settingTitle: _languageTitle
                imageLeftSrc: "qrc:/ui/resources/settings/language.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.languageSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewLanguage.qml")
                }
                visible: MZLocalizer.hasLanguages
            }

            MZSettingsItem {
                objectName: "dnsSettings"

                settingTitle: MZI18n.SettingsDnsSettings
                imageLeftSrc: "qrc:/ui/resources/settings/dnssettings.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.dnsSettingsSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewDNSSettings.qml")
                }
            }
        }
    }
}
