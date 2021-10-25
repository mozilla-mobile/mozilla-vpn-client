/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Window 2.12

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1
import themes 0.1

import org.mozilla.Glean 0.23
import telemetry 0.23

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

    //These need to be bound before onComplete so that the window buttons, menus and title bar double click behave properly
    maximumWidth: fullscreenRequired() ? Screen.width : Theme.desktopAppWidth;
    maximumHeight: fullscreenRequired() ? Screen.height : Theme.desktopAppHeight;

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
            minimumHeight = Theme.desktopAppHeight
            minimumWidth = Theme.desktopAppWidth

        }
        VPN.mainWindowLoaded()
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
        anchors.top: parent.top

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
        visible: isWasmApp
        height: Theme.menuHeight
        anchors.top: parent.top
        anchors.topMargin: iosSafeAreaTopMargin.height
    }

    VPNStackView {
        id: mainStackView
        initialItem: mainView
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: iosSafeAreaTopMargin.height + isWasmApp ? wasmMenuHeader.height : 0
        height: safeContentHeight
        clip: true
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
                        source: VPNFeatureList.get("inAppAuthentication").isSupported ? "states/StateAuthenticationInApp.qml" : "states/StateAuthenticating.qml"
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
                    name: VPN.StateSubscriptionInProgress

                    PropertyChanges {
                        target: loader
                        source: "states/StateSubscriptionInProgress.qml"
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

                },
                State {
                    name: VPN.StateBillingNotAvailable

                    PropertyChanges {
                        target: loader
                        source: "states/StateBillingNotAvailable.qml"
                    }
                },
                State {
                    name: VPN.StateSubscriptionNotValidated

                    PropertyChanges {
                        target: loader
                        source: "states/StateSubscriptionNotValidated.qml"
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
            if (VPNFeatureList.get("shareLogs").isSupported)  {
                if(VPN.viewLogs()){
                    return;
                };
            }
            // If we cant show logs natively, open the viewer
            mainStackView.push("views/ViewLogs.qml");
            
        }

        function onContactUsNeeded() {
            mainStackView.push("views/ViewContactUs.qml");
        }

        function onLoadAndroidAuthenticationView() {
            if (Qt.platform.os !== "android") {
                console.log("Unexpected android authentication view request!");
            }

            mainStackView.push("qrc:/ui/platforms/android/androidauthenticationview.qml", StackView.Immediate)
        }

        function onInitializeGlean() {
            var debug = {};
            if (VPN.debugMode) {
                console.debug("Initializing glean with debug mode");
                debug = {
                    logPings: true,
                    debugViewTag: "MozillaVPN"
                };
            }
            var channel = VPN.stagingMode ? "staging" : "production";
            console.debug("Initializing glean with channel set to:", channel);
            Glean.initialize("mozillavpn", VPNSettings.gleanEnabled, {
                appBuild: "MozillaVPN/" + VPN.versionString,
                appDisplayVersion: VPN.versionString,
                channel: channel,
                debug: debug,
                osVersion: VPN.osVersion,
                architecture: VPN.architecture,
            });
            if (VPN.platform == "dummy") {
                console.debug("Setting glean tags for automation");
                Glean.setSourceTags(["automation"])
            }
        }

        function onSendGleanPings() {
            console.debug("sending Glean pings");
            Pings.main.submit();
        }

        function onRecordGleanEvent(sample) {
            console.debug("recording Glean event");
            Sample[sample].record();
        }

        function onAboutToQuit() {
            console.debug("about to quit, shutdown Glean");
            // Use glean's built-in shutdown method - https://mozilla.github.io/glean/book/reference/general/shutdown.html
            Glean.shutdown();
        }
    }

    Connections {
        target: VPNSettings
        function onGleanEnabledChanged() {
            console.debug("Glean - onGleanEnabledChanged", VPNSettings.gleanEnabled);
            Glean.setUploadEnabled(VPNSettings.gleanEnabled);
        }
    }

    VPNSystemAlert {
    }

    VPNFeatureTourPopup {
        id: featureTourPopup

        Component.onCompleted: {
            featureTourPopup.handleShowTour();
        }

        function handleShowTour() {
            if(VPN.state === VPN.StateMain
                && !VPNSettings.featuresTourShown
                && VPNWhatsNewModel.hasUnseenFeature
            ) {
                featureTourPopup.openTour();
            }
        }
    }
}
