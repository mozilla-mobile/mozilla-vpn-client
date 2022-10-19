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

    anchors.fill: parent

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
                    Layout.preferredWidth: VPNTheme.theme.vSpacing
                    Layout.preferredHeight: VPNTheme.theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: VPNSettings.dnsProvider == settingValue
                    ButtonGroup.group: radioButtonGroup
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

                    VPNVerticalSpacer {
                        visible: ipInput.visible
                        Layout.preferredHeight: VPNTheme.theme.windowMargin
                        width: undefined
                    }

                    VPNTextField {
                        property bool valueInvalid: false
                        property string error: "This is an error string"
                        hasError: valueInvalid
                        visible: showDNSInput
                        id: ipInput

                        enabled: (VPNSettings.dnsProvider === VPNSettings.Custom) && vpnIsOff
                        _placeholderText: VPNSettings.placeholderUserDNS
                        text: ""
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40

                        PropertyAnimation on opacity {
                            duration: 200
                        }

                        Component.onCompleted: {
                            ipInput.text = VPNSettings.userDNS;
                        }

                        onTextChanged: text => {
                            if (ipInput.text === "") {
                                // If nothing is entered, thats valid too. We will ignore the value later.
                                ipInput.valueInvalid = false;
                                VPNSettings.userDNS = ipInput.text
                                return;
                            }
                            if (VPN.validateUserDNS(ipInput.text)) {
                                ipInput.valueInvalid = false;
                                VPNSettings.userDNS = ipInput.text
                            } else {
                                ipInput.error = VPNl18n.CustomDNSSettingsInlineCustomDNSError
                                ipInput.valueInvalid = true;
                            }
                        }
                    }

                    VPNContextualAlerts {
                        id: errorAlert

                        anchors {
                            left: undefined
                            right: undefined
                        }
                        Layout.topMargin: VPNTheme.theme.listSpacing

                        visible: ipInput.valueInvalid && ipInput.visible

                        messages: [
                            {
                                type: "error",
                                message: ipInput.error,
                                visible: ipInput.valueInvalid && ipInput.visible
                            }
                        ]
                    }

                }
            }
        }
    }
}
