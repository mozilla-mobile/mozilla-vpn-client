/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: vpnFlickable
    objectName: "settingsNotifications"

    //% "Notifications"
    _menuTitle: qsTrId("vpn.settings.notifications")

    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin * 1.5

        MZCheckBoxRow {
            id: captivePortalAlert
            objectName: "settingCaptivePortalAlert"
            visible: MZFeatureList.get("captivePortal").isSupported
            //% "Guest Wi-Fi portal alert"
            labelText: qsTrId("vpn.settings.guestWifiAlert")
            //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
            subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")
            isChecked: (MZSettings.captivePortalAlert)
            showDivider: false
            onClicked: {
                MZSettings.captivePortalAlert = !MZSettings.captivePortalAlert
            }
            Layout.rightMargin: MZTheme.theme.windowMargin
        }

        MZCheckBoxRow {
            id: unsecuredNetworkAlert
            objectName: "settingUnsecuredNetworkAlert"
            visible: MZFeatureList.get("unsecuredNetworkNotification").isSupported
            //% "Unsecured network alert"
            labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
            //% "Get notified if you connect to an unsecured Wi-Fi network"
            subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")
            isChecked: (MZSettings.unsecuredNetworkAlert)
            enabled: true
            showDivider: !enabled
            onClicked: {
                MZSettings.unsecuredNetworkAlert = !MZSettings.unsecuredNetworkAlert
            }
            Layout.rightMargin: MZTheme.theme.windowMargin
        }

        MZCheckBoxRow {
            id: switchServersAlert
            objectName: "switchServersAlert"
            visible: MZFeatureList.get("notificationControl").isSupported
            //% "Server switching notification"
            labelText: qsTrId("vpn.settings.notification.serverSwitch2")
            //% "Get notified when you successfully switched servers"
            subLabelText: qsTrId("vpn.settings.notification.serverSwitch.description")
            isChecked: (MZSettings.serverSwitchNotification)
            showDivider: false
            onClicked: {
                MZSettings.serverSwitchNotification = !MZSettings.serverSwitchNotification
            }
            Layout.rightMargin: MZTheme.theme.windowMargin
        }

        MZCheckBoxRow {
            id: connectionChangeAlert
            objectName: "connectionChangeAlert"
            visible: MZFeatureList.get("notificationControl").isSupported

            //% "Connection change notification"
            labelText: qsTrId("vpn.settings.notification.connectionChange2")
            //% "Get notified when the connection status changes"
            subLabelText: qsTrId("vpn.settings.notification.connectionChange.description")
            isChecked: (MZSettings.connectionChangeNotification)
            showDivider: false
            onClicked: {
                MZSettings.connectionChangeNotification = !MZSettings.connectionChangeNotification
            }
            Layout.rightMargin: MZTheme.theme.windowMargin
        }

        MZCheckBoxRow {
            id: serverUnavailableNotification
            objectName: "serverUnavailableNotification"
            visible: MZFeatureList.get("serverUnavailableNotification").isSupported
            labelText: MZI18n.ServerUnavailableNotificationPreferencesLabel
            subLabelText: MZI18n.ServerUnavailableNotificationPreferencesSubLabel
            isChecked: (MZSettings.serverUnavailableNotification)
            showDivider: false
            onClicked: {
                MZSettings.serverUnavailableNotification = !MZSettings.serverUnavailableNotification
            }
            Layout.rightMargin: MZTheme.theme.windowMargin
        }
    }
    Component.onCompleted: {
        Glean.sample.notificationsViewOpened.record();
    }
}
