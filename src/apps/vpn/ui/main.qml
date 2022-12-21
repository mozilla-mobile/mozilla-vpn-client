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

    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height

    LayoutMirroring.enabled: VPNLocalizer.isRightToLeft
    LayoutMirroring.childrenInherit: true

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
        case 2556: // iPhone_14_Pro
            return 48;
        case 2796: // iPhone_14_Pro_Max
            return 48;
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
    color: VPNTheme.theme.bgColor
    onClosing: close => {
        console.log("Closing request handling");

        // No desktop, we go in background mode.
        if (!fullscreenRequired()) {
            close.accepted = false;
            window.hide();
            return;
        }

        if (VPNNavigator.eventHandled()) {
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

    VPNNavigatorLoader {
      objectName: "screenLoader"
      anchors {
          top: iosSafeAreaTopMargin.bottom
          left: parent.left
          right: parent.right
          bottom: parent.bottom
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

            VPNNavigator.requestScreen(VPNNavigator.ScreenViewLogs);
        }

        function onAccountDeleted() {
            VPNController.logout();
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
                // Uncomment for debugging purposes.
                // See: https://mozilla.github.io/glean/book/reference/debug/debugViewTag.html#debug-view-tag
                //
                // Glean.setDebugViewTag("MozillaVPN");
            }
            var channel = VPN.stagingMode ? "staging" : "production";

            console.debug("Initializing glean with channel set to:", channel);
            Glean.initialize("mozillavpn", VPNSettings.gleanEnabled, {
                appBuild: "MozillaVPN/" + VPN.env.versionString,
                appDisplayVersion: VPN.env.versionString,
                channel: channel,
                osVersion: VPN.env.osVersion,
                architecture: [VPN.env.architecture, VPN.env.graphicsApi].join(" ").trim(),
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
            // Submit the main ping in case there are outstading metrics in storage before shutdown.
            Pings.main.submit();
            // Use glean's built-in shutdown method - https://mozilla.github.io/glean/book/reference/general/shutdown.html
            Glean.shutdown();
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

    VPNTutorialPopups {
        id: tutorialUI
    }

    VPNSystemAlert {
    }

    VPNServerUnavailablePopup {
        id: serverUnavailablePopup
    }

    Connections {
        target: VPNController
        function onReadyToServerUnavailable(receivedPing) {
            serverUnavailablePopup.receivedPing = receivedPing
            serverUnavailablePopup.open();
        }
    }

    VPNBottomNavigationBar {
        id: navbar
    }

}
