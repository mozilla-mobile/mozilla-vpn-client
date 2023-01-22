/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30


VPNFlickable {

    id: vpnFlickable
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
    property alias settingsListModel: repeater.model

    flickContentHeight: col.height + col.anchors.topMargin

    ColumnLayout {
        id: col
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: 18
        anchors.left: parent.left
        anchors.leftMargin: 18
        anchors.right: parent.right
        anchors.rightMargin: VPNTheme.theme.windowMargin
        spacing: VPNTheme.theme.vSpacing

        VPNCheckBoxAlert {
            id: alert
            //% "VPN must be off to edit these settings"
            //: Associated to a group of settings that require the VPN to be disconnected to change
            errorMessage: qsTrId("vpn.settings.vpnMustBeOff")
            anchors {
                left: undefined
                right: undefined
            }
        }

        Repeater {
            id: repeater

            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin

                VPNRadioButton {
                    id: radioButton
                    Layout.preferredWidth: VPNTheme.theme.vSpacing
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: VPNSettings.dnsProvider == settingValue
                    accessibleName: settingTitle
                    enabled: vpnIsOff
                    onClicked: VPNSettings.dnsProvider = settingValue
                }

                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    VPNInterLabel {
                        Layout.fillWidth: true

                        text: settingTitle
                        wrapMode: Text.WordWrap
                        opacity: vpnIsOff ? 1 : .5
                        horizontalAlignment: Text.AlignLeft

                        VPNMouseArea {
                            anchors.fill: undefined
                            anchors.top: parent.top
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom

                            enabled: radioButton.enabled
                            width: Math.min(parent.implicitWidth, parent.width)
                            propagateClickToParent: false
                            onClicked: VPNSettings.dnsProvider = settingValue
                        }
                    }

                    VPNTextBlock {
                        text: settingDescription
                        opacity: vpnIsOff ? 1 : .5

                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
