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
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin

        MZToggleRow {
            objectName: "settingCaptivePortalAlert"

            //% "Guest Wi-Fi portal alert"
            labelText: qsTrId("vpn.settings.guestWifiAlert")
            //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
            subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")
            checked: MZSettings.captivePortalAlert
            visible: MZFeatureList.get("captivePortal").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.captivePortalAlert = !MZSettings.captivePortalAlert
        }

        MZToggleRow {
            objectName: "settingUnsecuredNetworkAlert"

            //% "Unsecured network alert"
            labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
            //% "Get notified if you connect to an unsecured Wi-Fi network"
            subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")
            checked: MZSettings.unsecuredNetworkAlert
            visible: MZFeatureList.get("unsecuredNetworkNotification").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.unsecuredNetworkAlert = !MZSettings.unsecuredNetworkAlert
        }

        MZToggleRow {
            objectName: "switchServersAlert"

            //% "Server switching notification"
            labelText: qsTrId("vpn.settings.notification.serverSwitch2")
            //% "Get notified when you successfully switched servers"
            subLabelText: qsTrId("vpn.settings.notification.serverSwitch.description")
            checked: MZSettings.serverSwitchNotification
            visible: MZFeatureList.get("notificationControl").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.serverSwitchNotification = !MZSettings.serverSwitchNotification
        }

        MZToggleRow {
            objectName: "connectionChangeAlert"

            //% "Connection change notification"
            labelText: qsTrId("vpn.settings.notification.connectionChange2")
            //% "Get notified when the connection status changes"
            subLabelText: qsTrId("vpn.settings.notification.connectionChange.description")
            checked: MZSettings.connectionChangeNotification
            visible: MZFeatureList.get("notificationControl").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.connectionChangeNotification = !MZSettings.connectionChangeNotification
        }

        MZToggleRow {
            objectName: "serverUnavailableNotification"

            labelText: MZI18n.ServerUnavailableNotificationPreferencesLabel
            subLabelText: MZI18n.ServerUnavailableNotificationPreferencesSubLabel
            checked: MZSettings.serverUnavailableNotification
            visible: MZFeatureList.get("serverUnavailableNotification").isSupported
            dividerTopMargin: MZTheme.theme.toggleRowDividerSpacing
            onClicked: MZSettings.serverUnavailableNotification = !MZSettings.serverUnavailableNotification
        }
    }

    Component.onCompleted: {
        Glean.sample.notificationsViewOpened.record();
    }
}
