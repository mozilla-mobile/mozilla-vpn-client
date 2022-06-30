/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

Window {
    id: window

    signal showServerList

    property bool _fallbackQtQuickRenderer: QT_QUICK_BACKEND == "software" //TODO pending #3398
    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height
    property var isWasmApp: Qt.platform.os === "wasm"

    function fullscreenRequired() {
        return Qt.platform.os === "android" ||
                Qt.platform.os === "ios" ||
                Qt.platform.os === "tvos";
    }


    function safeAreaHeightByDevice() {
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

    screen: Qt.platform.os === "wasm" && Qt.application.screens.length > 1 ? Qt.application.screens[1] : Qt.application.screens[0]
    flags: Qt.platform.os === "ios" ? Qt.MaximizeUsingFullscreenGeometryHint : Qt.Window
    visible: true

    width: fullscreenRequired() ? Screen.width : VPNTheme.theme.desktopAppWidth;
    height: fullscreenRequired() ? Screen.height : VPNTheme.theme.desktopAppHeight;

    //These need to be bound before onComplete so that the window buttons, menus and title bar double click behave properly
    maximumWidth: fullscreenRequired() ? Screen.width : VPNTheme.theme.desktopAppWidth;
    maximumHeight: fullscreenRequired() ? Screen.height : VPNTheme.theme.desktopAppHeight;

    //% "Mozilla VPN"
    title: qsTrId("vpn.main.productName")
    color: "#F9F9FA"
    onClosing: close => {
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
            minimumHeight = VPNTheme.theme.desktopAppHeight
            minimumWidth = VPNTheme.theme.desktopAppWidth

        }
        VPN.mainWindowLoaded()
    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        z: 10
        onPressed: mouse => {
            if (window.activeFocusItem && window.activeFocusItem.forceBlurOnOutsidePress) {
                window.activeFocusItem.focus = false;
            }
            mouse.accepted = false;
        }
    }

    VPNMobileStatusBarModifier {
        id: statusBarModifier
    }

    Rectangle {
        id: iosSafeAreaTopMargin

        color: VPNTheme.theme.transparent
        height: safeAreaHeightByDevice();
        width: window.width
        anchors.top: parent.top
    }

    VPNWasmHeader {
        id: wasmMenuHeader
        visible: isWasmApp
        height: visible ? VPNTheme.theme.menuHeight : 0
        anchors.top: parent.top
        anchors.topMargin: iosSafeAreaTopMargin.height
    }

    VPNStackView {
        id: mainStackView

        objectName: "MainStackView"
        initialItem: mainView
        width: parent.width
        anchors.top: parent.top
        anchors.topMargin: iosSafeAreaTopMargin.height + wasmMenuHeader.height
        height: safeContentHeight

        function getHelpViewNeeded() {
            mainStackView.push("qrc:/ui/views/ViewGetHelp.qml")
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
                }
            }

            if (tutorialUI.visible) {
                return tutorialUI.openLeaveTutorialPopup(VPN.viewLogsNeeded)
            }

            // If we can't show logs natively, open the viewer
            if (mainStackView.currentItem.objectName !== "viewLogs") {
                mainStackView.push("qrc:/ui/views/ViewLogs.qml");
            }
        }
        function onContactUsNeeded() {
            if (tutorialUI.visible) {
                return tutorialUI.openLeaveTutorialPopup(VPN.contactUsNeeded)
            }

            // Check if Contact Us view is already in mainStackView
            const contactUsViewInStack = mainStackView.find((view) => { return view.objectName === "contactUs" });
            if (contactUsViewInStack) {
                // Unwind mainStackView back to Contact Us
                return mainStackView.pop(contactUsViewInStack, StackView.Immediate);
            }
            mainStackView.push("qrc:/ui/views/ViewContactUs.qml", StackView.Immediate);
        }

        function onSettingsNeeded() {
            if (tutorialUI.visible) {
                return tutorialUI.openLeaveTutorialPopup(VPN.settingsNeeded);
            }

            // Check if Settings view is already in mainStackView
            const settingsViewInMainStack = mainStackView.find((view) => { return view.objectName === "settings" })

            if (settingsViewInMainStack) {
                // Unwind settingsStackView back to menu
                settingsViewInMainStack._unwindSettingsStackView();

                // Unwind mainStackView back to Settings
                return mainStackView.pop(settingsViewInMainStack, StackView.Immediate);
            }
            mainStackView.push("qrc:/ui/views/ViewSettings.qml", StackView.Immediate);
        }
        function onAccountDeleted() {
            VPNController.logout();
            mainStackView.unwindToInitialItem();
        }
    }

    // Glean Connections
    Connections {
        target: VPN
        enabled: Qt.platform.os !== "android"

        function onInitializeGlean() {
            if (VPN.debugMode) {
                console.debug("Initializing glean with debug mode");
                Glean.setLogPings(true);
                Glean.setDebugViewTag("MozillaVPN");
            }
            var channel = VPN.stagingMode ? "staging" : "production";

            console.debug("Initializing glean with channel set to:", channel);
            Glean.initialize("mozillavpn", VPNSettings.gleanEnabled, {
                appBuild: "MozillaVPN/" + VPN.versionString,
                appDisplayVersion: VPN.versionString,
                channel: channel,
                osVersion: VPN.osVersion,
                architecture: [VPN.architecture, VPN.graphicsApi].join(" ").trim(),
            });
        }

        function onSetGleanSourceTags(tags) {
            console.debug("Setting source tags to:", tags);
            Glean.setSourceTags(tags);
        }

        function onSendGleanPings() {
            console.debug("sending Glean pings");
            Pings.main.submit();
        }

        function onRecordGleanEvent(sample) {
            console.debug("recording Glean event");
            Sample[sample].record();
        }

        function onRecordGleanEventWithExtraKeys(sample, extraKeys) {
            console.debug("recording Glean event with extra keys");
            Sample[sample].record(extraKeys);
        }

        function onAboutToQuit() {
            console.debug("about to quit, shutdown Glean");
            // Use glean's built-in shutdown method - https://mozilla.github.io/glean/book/reference/general/shutdown.html
            Glean.shutdown();
        }
    }

    Connections {
        target: VPNAddonManager
        function onRunAddon(addon) {
            console.log("Loading addon", addon.name);
            mainStackView.push("qrc:/ui/views/ViewAddon.qml", { addon })
        }
    }

    Connections {
        target: VPNSettings
        enabled: Qt.platform.os != "android"
        function onGleanEnabledChanged() {
            console.debug("Glean - onGleanEnabledChanged", VPNSettings.gleanEnabled);
            Glean.setUploadEnabled(VPNSettings.gleanEnabled);
        }
    }

    Connections {
        target: VPNErrorHandler
        function onSubscriptionGeneric() {
            if(VPN.state !== VPN.StateSubscriptionNeeded && VPN.state !== VPN.StateSubscriptionInProgress) {
                return;
            }

            mainStackView.push("qrc:/ui/views/ViewErrorFullScreen.qml", {
                // Problem confirming subscription...
                headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

                // Sorry, we were unable to confirm your subscription.
                // Please try again or contact our support team for help.
                errorMessage: VPNl18n.RestorePurchaseGenericPurchaseErrorRestorePurchaseGenericPurchaseErrorText,

                // Try again
                primaryButtonText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton,
                primaryButtonObjectName: "errorTryAgainButton",
                primaryButtonOnClick: mainStackView.pop,
                secondaryButtonIsSignOff: false,
                getHelpLinkVisible: true
            });
        }

        function onNoSubscriptionFound() {
            if(VPN.state !== VPN.StateSubscriptionNeeded && VPN.state !== VPN.StateSubscriptionInProgress) {
                return;
            }

            mainStackView.push("qrc:/ui/views/ViewErrorFullScreen.qml", {
                // Problem confirming subscription...
                headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

                // Sorry, we were unable to confirm your subscription.
                // Please try again or contact our support team for help.
                errorMessage: VPNl18n.RestorePurchaseGenericPurchaseErrorRestorePurchaseGenericPurchaseErrorText,

                // Try again
                primaryButtonText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton,
                primaryButtonObjectName: "errorTryAgainButton",
                primaryButtonOnClick: mainStackView.pop,
                secondaryButtonIsSignOff: true,
                getHelpLinkVisible: true,
                popWhenSignOff: true
            });
        }

        function onSubscriptionExpired() {
            if(VPN.state !== VPN.StateSubscriptionNeeded && VPN.state !== VPN.StateSubscriptionInProgress) {
                return;
            }

            mainStackView.push("qrc:/ui/views/ViewErrorFullScreen.qml", {
                // Problem confirming subscription...
                headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

                // Sorry we are unable to connect your Firefox Account to a current subscription.
                // Please try again or contact our support team for further assistance.
                errorMessage: VPNl18n.RestorePurchaseExpiredErrorRestorePurchaseExpiredErrorText,

                // Try again
                primaryButtonText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton,
                primaryButtonObjectName: "errorTryAgainButton",
                primaryButtonOnClick: mainStackView.pop,
                secondaryButtonIsSignOff: false,
                getHelpLinkVisible: true
            });
        }

        function onSubscriptionInUse() {
            if(VPN.state !== VPN.StateSubscriptionNeeded && VPN.state !== VPN.StateSubscriptionInProgress) {
                return;
            }

            mainStackView.push("qrc:/ui/views/ViewErrorFullScreen.qml", {
                // Problem confirming subscription...
                headlineText: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorHeader,

                // Another Firefox Account has already subscribed using this Apple ID.
                // Visit our help center below to learn more about how to manage your subscriptions.
                errorMessage: VPNl18n.RestorePurchaseInUseErrorRestorePurchaseInUseErrorText,

                // Sign out
                primaryButtonText: qsTrId("vpn.main.signOut2"),
                primaryButtonObjectName: "errorSignOutButton",
                primaryButtonOnClick: () => {
                    VPNController.logout();
                    mainStackView.pop();
                },
                secondaryButtonIsSignOff: false,
                getHelpLinkVisible: true
            });
        }

    }

    VPNTutorialPopups {
        id: tutorialUI
    }

    VPNSystemAlert {
    }

    VPNServerUnavailablePopup {
        id: serverUnavailablePopup
    }

    function goToServersView() {
        if (VPN.state !== VPN.StateMain) {
            return;
        }
        if (mainStackView.depth > 1) {
            mainStackView.unwindToInitialItem();
        }
        showServerList();
    }

    function exitTutorialIfNeeded() {
        if (tutorialUI.visible) {
            tutorialUI.leaveTutorial();
        }
    }

    function pushCaptivePortalView() {
        exitTutorialIfNeeded();
        mainStackView.push("qrc:/ui/views/ViewCaptivePortalInfo.qml", StackView.Immediate);
    }

    Connections {
        target: VPNController
        function onReadyToServerUnavailable() {
            exitTutorialIfNeeded();
            serverUnavailablePopup.open();
        }
        function onActivationBlockedForCaptivePortal() {
          pushCaptivePortalView();
        }
    }
    Connections{
        target: VPNCaptivePortal
        function onCaptivePortalPresent() {
            pushCaptivePortalView();
        }
    }

    VPNFeatureTourPopup {
        id: featureTourPopup

        anchors.centerIn: parent
    }
}
