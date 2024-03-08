/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZViewBase {
    id: vpnFlickable
    property string _startAtBootTitle: ""
    property string _notificationsTitle: ""
    property string _languageTitle: ""
    readonly property string telemetryScreenId : "app_preferences"

    objectName: "settingsPreferencesView"

    Component.onCompleted: Glean.impression.appPreferencesScreen.record({screen:telemetryScreenId})

    _viewContentData: Column {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        MZToggleRow {
            objectName: "startAtBootToogle"

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: MZTheme.theme.windowMargin
            anchors.rightMargin: MZTheme.theme.windowMargin

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

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: MZTheme.theme.windowMargin
            anchors.rightMargin: MZTheme.theme.windowMargin

            labelText: MZI18n.TelemetryPolicyViewDataCollectionAndUse
            subLabelText: MZI18n.SettingsDataCollectionDescription
            checked: MZSettings.gleanEnabled
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.gleanEnabled = !MZSettings.gleanEnabled
        }

        Column {
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
                    stackview.push("qrc:/ui/screens/settings/ViewNotifications.qml")
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
                    stackview.push("qrc:/ui/screens/settings/ViewLanguage.qml")
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
                    stackview.push("qrc:/ui/screens/settings/ViewDNSSettings.qml")
                }
                width: parent.width - MZTheme.theme.windowMargin
            }
        }
    }
}
