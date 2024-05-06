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

            labelText: MZI18n.TelemetryPolicyViewDataCollectionAndUse
            subLabelText: MZI18n.SettingsDataCollectionDescription
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

            labelText: MZI18n.LocalNetworkAccessLabel
            subLabelText: MZI18n.LocalNetworkAccessSubLabel
            checked: MZSettings.localNetworkAccess
            enabled: isIOS && vpnIsOff
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: {
                if (VPNController.StateOff) {
                    MZSettings.localNetworkAccess = !MZSettings.localNetworkAccess
                }
            }
        }

        MZContextualAlerts {
            Layout.topMargin: MZTheme.theme.listSpacing
            Layout.rightMargin: MZTheme.theme.windowMargin
            Layout.leftMargin: MZTheme.theme.windowMargin

            visible: isIOS && !vpnIsOff

            messages: [
                {
                    type: "warning",
                    message: MZI18n.LocalNetworkAccessVpnMustBeOff,
                }
            ]
        }

        ColumnLayout {
            spacing: MZTheme.theme.windowMargin / 2
            width: parent.width
            MZSettingsItem {
                objectName: "settingsNotifications"
                anchors.left: parent.left
                anchors.right: parent.right

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
                    stackview.push("qrc:/Mozilla/VPN/screens/settings/ViewNotifications.qml")
                }
                visible: MZFeatureList.get("captivePortal").isSupported || MZFeatureList.get("unsecuredNetworkNotification").isSupported || MZFeatureList.get("notificationControl").isSupported
                width: parent.width - MZTheme.theme.windowMargin
            }

            MZSettingsItem {
                objectName: "settingsLanguages"

                anchors.left: parent.left
                anchors.right: parent.right

                settingTitle: _languageTitle
                imageLeftSrc: "qrc:/ui/resources/settings/language.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.languageSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/Mozilla/VPN/screens/settings/ViewLanguage.qml")
                }
                visible: MZLocalizer.hasLanguages
                width: parent.width - MZTheme.theme.windowMargin
            }

            MZSettingsItem {
                objectName: "dnsSettings"

                anchors.left: parent.left
                anchors.right: parent.right

                settingTitle: MZI18n.SettingsDnsSettings
                imageLeftSrc: "qrc:/ui/resources/settings/dnssettings.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: MZLocalizer.isRightToLeft
                onClicked: {
                    Glean.interaction.dnsSettingsSelected.record({screen:telemetryScreenId})
                    stackview.push("qrc:/Mozilla/VPN/screens/settings/ViewDNSSettings.qml")
                }
                width: parent.width - MZTheme.theme.windowMargin
            }
        }
    }
}
