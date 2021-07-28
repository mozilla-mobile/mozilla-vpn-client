/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../components/forms"
import "../themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15


Item {
    id: root

    VPNMenu {
        id: menu
        objectName: "settingsAdvancedDNSSettingsBackButton"

        //% "Advanced DNS settings"
        title: qsTrId("vpn.settings.advancedDNSSettings.title2")
        isSettingsView: true
    }

    VPNFlickable {
        id: vpnFlickable
        property bool vpnIsOff: (VPNController.state === VPNController.StateOff)

        anchors.top: menu.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        height: root.height - menu.height
        flickContentHeight: col.childrenRect.height
        interactive: flickContentHeight > height

        Component.onCompleted: {
            Glean.sample.dnsSettingsViewOpened.record();
        }

        ButtonGroup {
            id: radioButtonGroup
        }

        ColumnLayout {
            id: col
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: 18
            anchors.left: parent.left
            anchors.leftMargin: 18
            anchors.right: parent.right
            anchors.rightMargin: Theme.windowMargin
            spacing: Theme.vSpacing

            RowLayout {
                Layout.fillWidth: true
                spacing: Theme.windowMargin
                Layout.rightMargin: Theme.windowMargin

                VPNRadioButton {
                    Layout.preferredWidth: Theme.vSpacing
                    Layout.preferredHeight: Theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: VPNSettings.useGatewayDNS
                    accessibleName: useDefaultLabel.text

                    onClicked: {
                        if (vpnFlickable.vpnIsOff) {
                            VPNSettings.useGatewayDNS = true
                        }
                    }
                }

                Column {
                    spacing: 4
                    Layout.fillWidth: true
                    VPNInterLabel {
                        id: useDefaultLabel
                        //% "Use default DNS"
                        text: qsTrId("vpn.advancedDNSSettings.gateway")
                        Layout.alignment: Qt.AlignTop
                    }

                    VPNTextBlock {

                       //% "Automatically use Mozilla VPN-protected DNS"
                       text: qsTrId("vpn.advancedDNSSettings.gateway.description")
                       width: parent.width
                    }
                }
            }

            ColumnLayout {
                spacing: Theme.windowMargin

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Theme.windowMargin
                    Layout.rightMargin: Theme.windowMargin

                    VPNRadioButton {
                        Layout.preferredWidth: Theme.vSpacing
                        Layout.preferredHeight: Theme.rowHeight
                        Layout.alignment: Qt.AlignTop
                        checked: !VPNSettings.useGatewayDNS
                        onClicked: VPNSettings.useGatewayDNS = false
                        accessibleName:  useLocalDNSLabel.text
                    }

                    Column {
                        spacing: 4
                        Layout.fillWidth: true
                        VPNInterLabel {
                            id: useLocalDNSLabel
                            //% "Use local DNS"
                            text: qsTrId("vpn.advancedDNSSettings.localDNS")
                            Layout.alignment: Qt.AlignTop
                        }

                        VPNTextBlock {
                           //% "Resolve website domain names using a DNS in your local network"
                           text: qsTrId("vpn.advancedDNSSettings.localDNS.resolveWebsiteDomainNames")
                           width: parent.width
                        }
                    }
                }

                VPNTextField {
                    property bool valueInvalid: false
                    property string error: "This is an error string"
                    hasError: valueInvalid

                    id: ipInput

                    enabled: !VPNSettings.useGatewayDNS
                    placeholderText: VPNSettings.placeholderUserDNS
                    text: VPNSettings.userDNS
                    Layout.fillWidth: true
                    Layout.leftMargin: 40

                    PropertyAnimation on opacity {
                        duration: 200
                    }

                    onTextChanged: text => {
                        if (ipInput.text === "") {
                            // If nothing is entered, thats valid too. We will ignore the value later.
                            ipInput.valueInvalid = false;
                            VPNSettings.userDNS = ipInput.text
                            return;
                        }

                        switch(VPNSettings.validateUserDNS(ipInput.text)) {
                        case VPNSettings.UserDNSOK:
                            ipInput.valueInvalid = false;
                            if (text !== VPNSettings.userDNS) {
                                VPNSettings.userDNS = ipInput.text
                            }
                            break;

                        // Now bother user if the ip is invalid :)
                        case VPNSettings.UserDNSInvalid:
                            //% "Invalid IP address"
                            ipInput.error = qsTrId("vpn.settings.userDNS.invalid")
                            ipInput.valueInvalid = true;
                            break;

                        case VPNSettings.UserDNSOutOfRange:
                            //% "Out of range IP address"
                            ipInput.error = qsTrId("vpn.settings.userDNS.outOfRange")
                            ipInput.valueInvalid = true;
                            break;
                        }
                    }
                }

                VPNCheckBoxAlert {
                    id: errorAlert
                    errorMessage: ipInput.error
                    anchors.left: undefined
                    anchors.right: undefined
                    anchors.leftMargin: undefined
                    anchors.rightMargin: undefined
                    anchors.top: undefined
                    anchors.topMargin: undefined
                    Layout.leftMargin: ipInput.Layout.leftMargin
                    alertColor: Theme.red

                    states: [
                        State {
                            name: "visible"
                            when: ipInput.valueInvalid
                            PropertyChanges {
                                target: errorAlert
                                visible: true
                                opacity: 1
                            }
                        },
                        State {
                            name: "hidden"
                            when: !ipInput.valueInvalid
                            PropertyChanges {
                                target: errorAlert
                                visible: false
                                opacity: 0
                            }
                        }
                    ]

                    transitions: [
                        Transition {
                            to: "hidden"
                            SequentialAnimation {
                                PropertyAnimation {
                                    target: errorAlert
                                    property: "opacity"
                                    to: 0
                                    duration: 100
                                }
                                PropertyAction {
                                    target: errorAlert
                                    property: "visible"
                                    value: false
                                }
                            }
                        },
                        Transition {
                            to: "visible"
                            SequentialAnimation {
                                PropertyAction {
                                    target: errorAlert
                                    property: "visible"
                                    value: true
                                }
                                PropertyAnimation {
                                    target: errorAlert
                                    property: "opacity"
                                    from: 0
                                    to: 1
                                    duration: 100
                                }
                            }
                        }
                    ]
                }
            }
        }
    }
}

