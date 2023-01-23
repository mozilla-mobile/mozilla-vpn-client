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
                    ButtonGroup.group: radioButtonGroup
                    accessibleName: settingTitle
                    enabled: true
                    onClicked: VPNSettings.dnsProvider = settingValue
                }

                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    VPNInterLabel {
                        Layout.fillWidth: true

                        text: settingTitle
                        wrapMode: Text.WordWrap
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
                        Layout.fillWidth: true
                    }

                    Loader {
                        Layout.fillWidth: true

                        active: showDNSInput
                        sourceComponent: ColumnLayout {

                            spacing: 0

                            VPNVerticalSpacer {
                                Layout.preferredHeight: VPNTheme.theme.windowMargin
                                width: undefined
                            }

                            VPNTextField {
                                id: ipInput

                                property bool valueInvalid: false
                                property string error: "This is an error string"


                                hasError: valueInvalid
                                visible: showDNSInput
                                enabled: (VPNSettings.dnsProvider === VPNSettings.Custom)
                                onEnabledChanged: if(enabled) forceActiveFocus()

                                _placeholderText: VPN.placeholderUserDNS
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

                                property string messageText: ""
                                property bool messageVisible: false

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
    }
}
