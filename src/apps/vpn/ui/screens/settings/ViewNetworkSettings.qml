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
    objectName: "settingsNetworkingBackButton"

    property string _appPermissionsTitle
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

    //% "Network settings"
    _menuTitle: qsTrId("vpn.settings.networking")
    _viewContentData: Column {
        id: col
        spacing: VPNTheme.theme.windowMargin
        Layout.fillWidth: true

        VPNContextualAlerts {
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: VPNTheme.theme.windowMargin
                rightMargin: VPNTheme.theme.windowMargin
            }

            messages: [
                {
                    type: "warning",
                    //% "VPN must be off to edit these settings"
                    //: Associated to a group of settings that require the VPN to be disconnected to change
                    message: qsTrId("vpn.settings.vpnMustBeOff"),
                    visible: VPNController.state !== VPNController.StateOff
                }
            ]
        }

        Column {
            width: parent.width
            spacing: VPNTheme.theme.windowMargin  /2
            VPNSettingsItem {
                objectName: "advancedDNSSettings"
                anchors.left: parent.left
                anchors.right: parent.right
                width: parent.width - VPNTheme.theme.windowMargin

                //% "Advanced DNS Settings"
                settingTitle: qsTrId("vpn.settings.networking.advancedDNSSettings")
                imageLeftSrc: "qrc:/ui/resources/settings-dark.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: VPNLocalizer.isRightToLeft
                onClicked: stackview.push("qrc:/ui/screens/settings/dnsSettings/ViewAdvancedDNSSettings.qml")
                visible: VPNFeatureList.get("customDNS").isSupported
            }

            VPNSettingsItem {
                objectName: "appPermissions"
                anchors.left: parent.left
                anchors.right: parent.right
                width: parent.width - VPNTheme.theme.windowMargin
                settingTitle: _appPermissionsTitle
                imageLeftSrc: "qrc:/ui/resources/settings/apps.svg"
                imageRightSrc: "qrc:/nebula/resources/chevron.svg"
                imageRightMirror: VPNLocalizer.isRightToLeft
                onClicked: stackview.push("qrc:/ui/screens/settings/appPermissions/ViewAppPermissions.qml")
                visible: VPNFeatureList.get("splitTunnel").isSupported
            }
        }
    }

    Component.onCompleted: {
        VPNGleanDeprecated.recordGleanEvent("networkSettingsViewOpened");
        Glean.sample.networkSettingsViewOpened.record();
        if (!vpnIsOff) {
            VPNGleanDeprecated.recordGleanEvent("networkSettingsViewWarning");
            Glean.sample.networkSettingsViewWarning.record();
        }
    }
}
