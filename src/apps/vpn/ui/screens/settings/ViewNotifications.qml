/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1


VPNViewBase {
    id: vpnFlickable
    objectName: "settingsNotifications"

    //% "Notifications"
    _menuTitle: qsTrId("vpn.settings.notifications")

    _viewContentData: ColumnLayout {
        Layout.preferredWidth: parent.width - VPNTheme.theme.windowMargin
        Layout.rightMargin: VPNTheme.theme.windowMargin
        spacing: VPNTheme.theme.windowMargin

        InformationCard {
            objectName: "viewNotificationsInformationCard"
            Layout.topMargin: VPNTheme.theme.windowMargin
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin
            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.windowMargin * 2
            Layout.minimumHeight: textBlocks.height
            visible: Qt.platform.os === "ios" && VPNController.state !== VPNController.StateOff

            _infoContent: ColumnLayout {
                id: textBlocks
                VPNTextBlock {
                    Layout.fillWidth: true
                    text: VPNl18n.SettingsIOSDisconnectWarning
                }
            }
        }

        VPNCheckBoxRow {
            id: captivePortalAlert
            objectName: "settingCaptivePortalAlert"
            visible: VPNFeatureList.get("captivePortal").isSupported
            //% "Guest Wi-Fi portal alert"
            labelText: qsTrId("vpn.settings.guestWifiAlert")
            //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
            subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")
            isChecked: (VPNSettings.captivePortalAlert)
            showDivider: false
            onClicked: {
                VPNSettings.captivePortalAlert = !VPNSettings.captivePortalAlert
            }
        }

        VPNCheckBoxRow {
            id: unsecuredNetworkAlert
            objectName: "settingUnsecuredNetworkAlert"
            visible: VPNFeatureList.get("unsecuredNetworkNotification").isSupported
            //% "Unsecured network alert"
            labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
            //% "Get notified if you connect to an unsecured Wi-Fi network"
            subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")
            isChecked: (VPNSettings.unsecuredNetworkAlert)
            enabled: true
            showDivider: !enabled
            onClicked: {
                VPNSettings.unsecuredNetworkAlert = !VPNSettings.unsecuredNetworkAlert
            }
        }

        VPNCheckBoxRow {
            id: switchServersAlert
            objectName: "switchServersAlert"
            visible: VPNFeatureList.get("notificationControl").isSupported
            //% "Server switching notification"
            labelText: qsTrId("vpn.settings.notification.serverSwitch2")
            //% "Get notified when you successfully switched servers"
            subLabelText: qsTrId("vpn.settings.notification.serverSwitch.description")
            isChecked: (VPNSettings.serverSwitchNotification)
            showDivider: false
            onClicked: {
                VPNSettings.serverSwitchNotification = !VPNSettings.serverSwitchNotification
            }
        }

        VPNCheckBoxRow {
            id: connectionChangeAlert
            objectName: "connectionChangeAlert"
            visible: VPNFeatureList.get("notificationControl").isSupported

            //% "Connection change notification"
            labelText: qsTrId("vpn.settings.notification.connectionChange2")
            //% "Get notified when the connection status changes"
            subLabelText: qsTrId("vpn.settings.notification.connectionChange.description")
            isChecked: (VPNSettings.connectionChangeNotification)
            showDivider: false
            onClicked: {
                VPNSettings.connectionChangeNotification = !VPNSettings.connectionChangeNotification
            }
        }

        VPNCheckBoxRow {
            id: serverUnavailableNotification
            objectName: "serverUnavailableNotification"
            visible: VPNFeatureList.get("serverUnavailableNotification").isSupported
            labelText: VPNl18n.ServerUnavailableNotificationPreferencesLabel
            subLabelText: VPNl18n.ServerUnavailableNotificationPreferencesSubLabel
            isChecked: (VPNSettings.serverUnavailableNotification)
            showDivider: false
            onClicked: {
                VPNSettings.serverUnavailableNotification = !VPNSettings.serverUnavailableNotification
            }
        }
    }
    Component.onCompleted: {
        VPNGleanDeprecated.recordGleanEvent("notificationsViewOpened");
        Glean.sample.notificationsViewOpened.record();
    }
}
