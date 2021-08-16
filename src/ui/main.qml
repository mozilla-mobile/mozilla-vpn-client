/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Window 2.12
import Mozilla.VPN 1.0
import "./components"
import "themes/themes.js" as Theme

import org.mozilla.Glean 0.15
import telemetry 0.15

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

    width: fullscreenRequired() ? Screen.width : Theme.desktopAppWidth;
    height: fullscreenRequired() ? Screen.height : Theme.desktopAppHeight;

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
        if (VPN.startMinimized) {
            this.showMinimized();
        }

        if (!fullscreenRequired()) {
            maximumHeight = Theme.desktopAppHeight
            minimumHeight = Theme.desktopAppHeight
            maximumWidth = Theme.desktopAppWidth
            minimumWidth = Theme.desktopAppWidth
        }

        Glean.initialize('MozillaVPN', VPNSettings.gleanEnabled, {
          appBuild: `MozillaVPN/${VPN.versionString}`,
          appDisplayVersion: VPN.versionString,
          httpClient: {
                  post(url, body, headers) {
                      if (typeof(VPNGleanTest) !== "undefined") {
                          VPNGleanTest.requestDone(url, body);
                      }
                      if (VPN.stagingMode) {
                          return Promise.reject('Glean disabled in staging mode');
                      }

                      return new Promise((resolve, reject) => {
                          const xhr = new XMLHttpRequest();
                          xhr.open("POST", url);

                          for (const header in headers) {
                            xhr.setRequestHeader(header, headers[header]);
                          }
                          xhr.onloadend = () => {
                            resolve({status: xhr.status, result: 2 /* UploadResultStatus.Success */ });
                          }
                          xhr.send(body);

                      });
                  }
          }
        });
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        z: 10
        onPressed: {
            if (window.activeFocusItem && window.activeFocusItem.forceBlurOnOutsidePress) {
                window.activeFocusItem.focus = false;
            }
            mouse.accepted = false;
        }
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
        clip: true

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
                        source: VPNFeatureList.authenticationInApp ? "states/StateAuthenticationInApp.qml" : "states/StateAuthenticating.qml"
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
                    name: VPN.StateTelemetryPolicy

                    PropertyChanges {
                        target: loader
                        source: "states/StateTelemetryPolicy.qml"
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

                },
                State {
                    name: VPN.StateBackendFailure

                    PropertyChanges {
                        target: loader
                        source: "states/StateBackendFailure.qml"
                    }

                }
            ]

            Loader {
                id: loader

                asynchronous: true
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

        function onSendGleanPings() {
            if (VPNSettings.gleanEnabled) {
                Pings.main.submit();
            }
        }

        function onTriggerGleanSample(sample) {
            Sample[sample].record();
        }

        function onAboutToQuit() {
            // We are about to quit. Let's see if we are fast enough to send
            // the last chunck of data to the glean servers.
            if (VPNSettings.gleanEnabled) {
              Pings.main.submit();
            }
        }
    }

    Connections {
        target: VPNSettings
        function onGleanEnabledChanged() {
            Glean.setUploadEnabled(VPNSettings.gleanEnabled);
        }
    }

    VPNSystemAlert {
    }

    VPNPopup {
        id: popupTest

        contentItem: VPNFeatureTour {
            onClose: {
            }
            onStarted: {
            }
            onFinished: {
            }
        }
    }

    Button {
        text: "Open"
        onClicked: popupTest.open()

        Component.onCompleted: {
            popupTest.open();
        }
    }
}
