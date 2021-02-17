/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Window 2.12
import Mozilla.VPN 1.0
import "./components"

Window {
    id: window

    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height
    property var isWasmApp: Qt.platform.os === "wasm"

    function fullscreenRequired() {
        return Qt.platform.os === "android" ||
                Qt.platform.os === "ios" ||
                Qt.platform.os === "tvos";
    }

    screen: Qt.platform.os === "wasm" && Qt.application.screens.length > 1 ? Qt.application.screens[1] : Qt.application.screens[0]

    flags: Qt.platform.os === "ios" ? Qt.MaximizeUsingFullscreenGeometryHint : Qt.Window

    visible: true
    width: fullscreenRequired() ? maximumWidth : 360
    height: fullscreenRequired() ? maximumHeight : 454
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    //% "Mozilla VPN"
    title: qsTrId("vpn.main.productName")
    color: "#F9F9FA"
    onClosing: {
        console.log("Closing request handling");

        // No desktop, we go in background mode.
        if (!fullscreenRequired()) {
            close.accepted = false;
            window.hide();
            return;
        }

        if (VPNCloseEventHandler.eventHandled()) {
            close.accepted = false;
            return;
        }

        console.log("closing.");
    }
    Component.onCompleted: {
        if (VPN.startMinimized)
            this.showMinimized();

    }
    Rectangle {
        id: iosSafeAreaTopMargin

        color: "transparent"
        height: marginHeightByDevice()
        width: window.width

        function marginHeightByDevice() {
            if (Qt.platform.os !== "ios") {
                return 0;
            }
            switch(window.height * Screen.devicePixelRatio) {
            case 1624: // iPhone_XR (Qt Provided Physical Resolution)
            case 1792: // iPhone_XR

            case 2436: // iPhone_X_XS
            case 2688: // iPhone_XS_MAX

            case 2532: // iPhone_12_Pro
            case 2778: // iPhone_12_Pro_Max
            case 2340: // iPhone_12_mini
                return 34;
            default:
                return 20;
            }

        }
    }

    VPNWasmHeader {
        id: wasmMenuHeader
        visible: false
    }

    VPNStackView {
        id: mainStackView
        initialItem: mainView
        width: parent.width
        anchors.top: iosSafeAreaTopMargin.bottom
        height: safeContentHeight

        Component.onCompleted: {
            if (isWasmApp) {
                wasmMenuHeader.visible = true;
                anchors.top = wasmMenuHeader.bottom;
                height = safeContentHeight - wasmMenuHeader.height;
            }
        }
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

                },
                State {
                    name: VPN.StateSubscriptionValidation

                    PropertyChanges {
                        target: loader
                        source: "states/StateSubscriptionValidation.qml"
                    }

                },
                State {
                    name: VPN.StateSubscriptionBlocked

                    PropertyChanges {
                        target: loader
                        source: "states/StateSubscriptionBlocked.qml"
                    }

                },
                State {
                    name: VPN.StateDeviceLimit

                    PropertyChanges {
                        target: loader
                        source: "states/StateDeviceLimit.qml"
                    }

                }
            ]

            Loader {
                id: loader

                anchors.fill: parent
            }

        }

    }


    Connections {
        target: VPN
        function onViewLogsNeeded() {
            if (Qt.platform.os !== "android" &&
                    Qt.platform.os !== "ios" &&
                    Qt.platform.os !== "tvos" &&
                    Qt.platform.os !== "wasm")  {
                VPN.viewLogs();
            } else {
                mainStackView.push("views/ViewLogs.qml");
            }
        }

        function onLoadAndroidAuthenticationView() {
            if (Qt.platform.os !== "android") {
                console.log("Unexpected android authentication view request!");
            }

            mainStackView.push("../platforms/android/androidauthenticationview.qml", StackView.Immediate)
        }
    }

    VPNSystemAlert {
        id: alertBox

        z: 2
    }

}
