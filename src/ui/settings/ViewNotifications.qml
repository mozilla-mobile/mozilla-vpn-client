/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root


    VPNMenu {
        id: menu
        objectName: "settingsNotificationsBackButton"

        //% "Notifications"
        title: qsTrId("vpn.settings.notifications")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        height: root.height - menu.height
        width: root.width
        anchors.left: root.left
        anchors.right: root.right
        anchors.top: menu.bottom
        flickContentHeight: col.y + col.childrenRect.height + ( Theme.rowHeight * 2 )
        interactive: flickContentHeight > height
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

        Component.onCompleted: {
            Sample.notificationsViewOpened.record();
            if (!vpnIsOff) {
                Sample.notificationsViewWarning.record();
            }
         }

        Column {
            id: col
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: Theme.windowMargin
            anchors.top: parent.top
            anchors.topMargin: Theme.windowMargin
            spacing: Theme.windowMargin

            VPNCheckBoxAlert {
                //% "VPN must be off to edit these settings"
                //: Associated to a group of settings that require the VPN to be disconnected to change
                errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
            }

            VPNCheckBoxRow {
                id: captivePortalAlert
                objectName: "settingCaptivePortalAlert"
                width: parent.width
                visible: VPNFeatureList.captivePortalNotificationSupported
                //% "Guest Wi-Fi portal alert"
                labelText: qsTrId("vpn.settings.guestWifiAlert")
                //% "Get notified if a guest Wi-Fi portal is blocked due to VPN connection"
                subLabelText: qsTrId("vpn.settings.guestWifiAlert.description")
                isChecked: (VPNSettings.captivePortalAlert)
                isEnabled: vpnFlickable.vpnIsOff
                showDivider: isEnabled
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.captivePortalAlert = !VPNSettings.captivePortalAlert
                    }
               }
            }

            VPNCheckBoxRow {
                id: unsecuredNetworkAlert
                objectName: "settingUnsecuredNetworkAlert"
                width: parent.width
                visible: VPNFeatureList.unsecuredNetworkNotificationSupported

                //% "Unsecured network alert"
                labelText: qsTrId("vpn.settings.unsecuredNetworkAlert")
                //% "Get notified if you connect to an unsecured Wi-Fi network"
                subLabelText: qsTrId("vpn.settings.unsecuredNetworkAlert.description")
                isChecked: (VPNSettings.unsecuredNetworkAlert)
                isEnabled: vpnFlickable.vpnIsOff
                showDivider: isEnabled
                onClicked: {
                    if (vpnFlickable.vpnIsOff) {
                        VPNSettings.unsecuredNetworkAlert = !VPNSettings.unsecuredNetworkAlert
                    }
               }
            }

            VPNCheckBoxRow {
                id: switchServersAlert
                objectName: "switchServersAlert"
                visible: VPNFeatureList.notificationControlSupported
                width: parent.width

                //% "Server Switching Notification"
                labelText: qsTrId("vpn.settings.notification.serverSwitch")
                //% "Get notified when you successfully switched servers"
                subLabelText: qsTrId("vpn.settings.notification.serverSwitch.description")
                isChecked: (VPNSettings.serverSwitchNotification)
                onClicked: {
                    VPNSettings.serverSwitchNotification = !VPNSettings.serverSwitchNotification
               }
            }

            VPNCheckBoxRow {
                id: connectionChangeAlert
                objectName: "connectionChangeAlert"
                visible: VPNFeatureList.notificationControlSupported
                width: parent.width

                //% "Connection Change Notification"
                labelText: qsTrId("vpn.settings.notification.connectionChange")
                //% "Get notified when the connection status changes"
                subLabelText: qsTrId("vpn.settings.notification.connectionChange.description")
                isChecked: (VPNSettings.connectionChangeNotification)
                onClicked: {
                    VPNSettings.connectionChangeNotification = !VPNSettings.connectionChangeNotification
                }
            }
        }
    }
}
