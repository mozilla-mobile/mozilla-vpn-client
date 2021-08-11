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


VPNFlickable {

    id: vpnFlickable
    property bool vpnIsOff: (VPNController.state === VPNController.StateOff)
    property alias settingsListModel: repeater.model


    function isChecked(settingId) {
        if (VPNSettings.useGatewayDNS && settingId === settingId_default)   {
            return true;
        }
        return false;
    }

    function handleClick(settingId) {
        if (!vpnFlickable.vpnIsOff) {
            return;
        }
        switch(String(settingId)) {
          case settingId_default.toString():
            console.log("User selected default DNS");
            VPNSettings.useGatewayDNS = true;
            break;
          case settingId_adblock.toString():
              console.log("User selected adblock");
              break;
          case settingId_antiTracking.toString():
              console.log("User selected antiTracking");
              break;
          case settingId_adblockAndAntiTracking.toString():
              console.log("User selected adblockAndAntiTracking");
              break;
          case settingId_local.toString():
            console.log("User selected local");
            VPNSettings.useGatewayDNS = false;
            break;
          default:
            console.log("TODO")
        }
        return;
    }
    flickContentHeight: col.height + 56*2
    interactive: flickContentHeight > height

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

        Repeater {
            id: repeater

            delegate: RowLayout {
                Layout.fillWidth: true
                spacing: Theme.windowMargin
                Layout.rightMargin: Theme.windowMargin

                VPNRadioButton {
                    Layout.preferredWidth: Theme.vSpacing
                    Layout.preferredHeight: Theme.rowHeight
                    Layout.alignment: Qt.AlignTop
                    checked: isChecked(settingId)
                    ButtonGroup.group: radioButtonGroup
                    accessibleName: settingTitle
                    onClicked: handleClick(settingId)
                }

                Column {
                    spacing: 4
                    Layout.fillWidth: true

                    VPNInterLabel {
                        text: settingTitle
                        wrapMode: Text.WordWrap
                        width: parent.width
                        horizontalAlignment: Text.AlignLeft
                    }

                    VPNTextBlock {
                       text: settingDescription
                       width: parent.width
                    }

                    VPNVerticalSpacer {
                        visible: ipInput.visible
                        height: Theme.windowMargin
                    }

                    VPNTextField {
                        property bool valueInvalid: false
                        property string error: "This is an error string"
                        hasError: valueInvalid
                        visible: showDNSInput
                        id: ipInput

                        enabled: !VPNSettings.useGatewayDNS
                        placeholderText: VPNSettings.placeholderUserDNS
                        text: VPNSettings.userDNS
                        width: parent.width
                        height: 40

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
                            if(VPNSettings.validateUserDNS(ipInput.text)){
                                ipInput.valueInvalid = false;
                                if (text !== VPNSettings.userDNS) {
                                    VPNSettings.userDNS = ipInput.text
                                }
                            }else{
                                // Now bother user if the ip is invalid :)
                                //% "Invalid IP address"
                                ipInput.error = qsTrId("vpn.settings.userDNS.invalid")
                                ipInput.valueInvalid = true;
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

}
