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

    objectName: "settingsPreferencesView"

    _viewContentData: Column {
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        MZCheckBoxRow {
            id: startAtBootCheckBox
            objectName: "settingStartAtBoot"

            labelText: _startAtBootTitle
            subLabelText: MZI18n.SettingsStartAtBootSubtitle
            isChecked: MZSettings.startAtBoot
            showDivider: false
            onClicked: MZSettings.startAtBoot = !MZSettings.startAtBoot
            visible: MZFeatureList.get("startOnBoot").isSupported
            anchors {
                right: parent.right
                left: parent.left
                rightMargin: MZTheme.theme.windowMargin
            }
        }

        MZCheckBoxRow {
            id: dataCollection
            objectName: "dataCollection"
            width: parent.width - MZTheme.theme.windowMargin
            anchors {
                right: parent.right
                left: parent.left
                rightMargin: MZTheme.theme.windowMargin
            }


            labelText: MZI18n.TelemetryPolicyViewDataCollectionAndUse
            subLabelText: MZI18n.SettingsDataCollectionDescription
            isChecked: MZSettings.gleanEnabled
            onClicked: {
                MZSettings.gleanEnabled = !MZSettings.gleanEnabled
           }
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
                onClicked: stackview.push("qrc:/ui/screens/settings/ViewLanguage.qml")
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
                onClicked: stackview.push("qrc:/ui/screens/settings/ViewDNSSettings.qml")
                width: parent.width - MZTheme.theme.windowMargin
            }
        }
    }
}
