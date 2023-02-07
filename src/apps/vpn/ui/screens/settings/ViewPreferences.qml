/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNViewBase {
    id: vpnFlickable
    property string _startAtBootTitle: ""
    property string _notificationsTitle: ""
    property string _languageTitle: ""

    objectName: "settingsPreferencesView"

    _viewContentData: Column {
        spacing: VPNTheme.theme.windowMargin
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter

        VPNCheckBoxRow {
            id: startAtBootCheckBox
            objectName: "settingStartAtBoot"

            labelText: _startAtBootTitle
            subLabelText: VPNl18n.SettingsStartAtBootSubtitle
            isChecked: VPNSettings.startAtBoot
            showDivider: false
            onClicked: VPNSettings.startAtBoot = !VPNSettings.startAtBoot
            visible: VPNFeatureList.get("startOnBoot").isSupported
            anchors {
                right: parent.right
                left: parent.left
                rightMargin: VPNTheme.theme.windowMargin
            }
        }

        VPNCheckBoxRow {
            id: dataCollection
            objectName: "dataCollection"
            width: parent.width - VPNTheme.theme.windowMargin
            anchors {
                right: parent.right
                left: parent.left
                rightMargin: VPNTheme.theme.windowMargin
            }

            //% "Data collection and use"
            labelText: qsTrId("vpn.settings.dataCollection")
            subLabelText: VPNl18n.SettingsDataCollectionDescription
            isChecked: VPNSettings.gleanEnabled
            onClicked: {
                VPNSettings.gleanEnabled = !VPNSettings.gleanEnabled
           }
        }

        Column {
            spacing: VPNTheme.theme.windowMargin / 2
            width: parent.width
            VPNSettingsItem {
                objectName: "settingsNotifications"
                anchors.left: parent.left
                anchors.right: parent.right

                settingTitle: _notificationsTitle
                imageLeftSrc: "qrc:/ui/resources/settings/notifications.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: VPNLocalizer.isRightToLeft
                onClicked: {
                    if(Qt.platform.os === "android"){
                        VPNAndroidUtils.openNotificationSettings();
                        return;
                    }
                    stackview.push("qrc:/ui/screens/settings/ViewNotifications.qml")
                }
                visible: VPNFeatureList.get("captivePortal").isSupported || VPNFeatureList.get("unsecuredNetworkNotification").isSupported || VPNFeatureList.get("notificationControl").isSupported
                width: parent.width - VPNTheme.theme.windowMargin
            }

            VPNSettingsItem {
                objectName: "settingsLanguages"

                anchors.left: parent.left
                anchors.right: parent.right

                settingTitle: _languageTitle
                imageLeftSrc: "qrc:/ui/resources/settings/language.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: VPNLocalizer.isRightToLeft
                onClicked: stackview.push("qrc:/ui/screens/settings/ViewLanguage.qml")
                visible: VPNLocalizer.hasLanguages
                width: parent.width - VPNTheme.theme.windowMargin
            }

            VPNSettingsItem {
                objectName: "dnsSettings"

                anchors.left: parent.left
                anchors.right: parent.right

                settingTitle: VPNl18n.SettingsDnsSettings
                imageLeftSrc: "qrc:/ui/resources/settings/dnssettings.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: VPNLocalizer.isRightToLeft
                onClicked: stackview.push("qrc:/ui/screens/settings/ViewDNSSettings.qml")
                width: parent.width - VPNTheme.theme.windowMargin
            }
        }
    }
}
