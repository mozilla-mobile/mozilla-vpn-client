/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Window 2.12
import Mozilla.VPN 1.0
import "./components"

Window {
    id: window

    visible: true
    width: 360
    height: 454
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    //% "Mozilla VPN"
    title: qsTrId("vpn.main.productName")
    color: "#F9F9FA"
    onClosing: {
        window.hide();
        close.accepted = false;
    }
    Component.onCompleted: {
        if (VPN.startMinimized)
            this.showMinimized();

    }

    StackView {
        id: mainStackView

        initialItem: mainView
        anchors.fill: parent
    }

    Component {
        id: mainView

        Item {
            state: VPN.state
            states: [
                State {
                    name: VPN.StateInitialize

                    PropertyChanges {
                        target: loader
                        source: "states/StateInitialize.qml"
                    }

                },
                State {
                    name: VPN.StateAuthenticating

                    PropertyChanges {
                        target: loader
                        source: "states/StateAuthenticating.qml"
                    }

                },
                State {
                    name: VPN.StatePostAuthentication

                    PropertyChanges {
                        target: loader
                        source: "states/StatePostAuthentication.qml"
                    }

                },
                State {
                    name: VPN.StateMain

                    PropertyChanges {
                        target: loader
                        source: "states/StateMain.qml"
                    }

                },
                State {
                    name: VPN.StateUpdateRequired

                    PropertyChanges {
                        target: loader
                        source: "states/StateUpdateRequired.qml"
                    }

                },
                State {
                    name: VPN.StateSubscriptionNeeded

                    PropertyChanges {
                        target: loader
                        source: "states/StateSubscriptionNeeded.qml"
                    }

                }
            ]

            Loader {
                id: loader

                anchors.fill: parent
            }

        }

    }

    VPNSystemAlert {
        id: alertBox

        z: 2
    }

}
