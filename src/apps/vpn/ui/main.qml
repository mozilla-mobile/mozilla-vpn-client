/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

Window {
    id: window

    signal showServerList
    signal screenClicked(double x, double y)

    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height

    LayoutMirroring.enabled: MZLocalizer.isRightToLeft
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

    function removeFocus(item, x, y) {
        //Remove focus if the global point pressed is not contained by the global area of the text field
        if(item.focus) {
            let globalX = item.mapToItem(window.contentItem, 0, 0).x
            let globalY = item.mapToItem(window.contentItem, 0, 0).y
            if(x < globalX || x > globalX + item.width || y < globalY || y > globalY + item.height)
                item.focus = false
        }
    }

    function repositionTutorialTooltip() {
        tutorialUI.repositionTutorialTooltip()
    }

    screen: Qt.platform.os === "wasm" && Qt.application.screens.length > 1 ? Qt.application.screens[1] : Qt.application.screens[0]
    flags: Qt.platform.os === "ios" ? Qt.MaximizeUsingFullscreenGeometryHint : Qt.Window
    visible: true

    width: fullscreenRequired() ? Screen.width : MZTheme.theme.desktopAppWidth;
    height: fullscreenRequired() ? Screen.height : MZTheme.theme.desktopAppHeight;

    //These need to be bound before onComplete so that the window buttons, menus and title bar double click behave properly
    maximumWidth: fullscreenRequired() ? Screen.width : MZTheme.theme.desktopAppWidth;
    maximumHeight: fullscreenRequired() ? Screen.height : MZTheme.theme.desktopAppHeight;

    //% "Mozilla VPN"
    title: qsTrId("vpn.main.productName")
    color: MZTheme.theme.bgColor
    onClosing: close => {
        console.log("Closing request handling");

        // No desktop, we go in background mode.
        if (!fullscreenRequired()) {
            close.accepted = false;
            window.hide();
            return;
        }

        if (MZNavigator.eventHandled()) {
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
            minimumHeight = MZTheme.theme.desktopAppHeight
            minimumWidth = MZTheme.theme.desktopAppWidth

        }
        VPN.mainWindowLoaded()
    }

    //Overlays the entire window at all times to remove focus from components on click away
    MouseArea {
        anchors.fill: parent
        z: MZTheme.theme.maxZLevel
        onPressed: (mouse) => {
            window.screenClicked(mouse.x, mouse.y)
            mouse.accepted = false
        }
    }

    MZMobileStatusBarModifier {
        id: statusBarModifier
    }

    Rectangle {
        id: iosSafeAreaTopMargin

        color: MZTheme.theme.transparent
        height: safeAreaHeightByDevice();
        width: window.width
        anchors.top: parent.top
    }

    MZNavigatorLoader {
      objectName: "screenLoader"
      anchors {
          top: iosSafeAreaTopMargin.bottom
          left: parent.left
          right: parent.right
          bottom: parent.bottom
      }
    }

    Connections {
        target: MZLog
        function onViewLogsNeeded() {
            if (MZFeatureList.get("shareLogs").isSupported)  {
                if(MZLog.viewLogs()){
                    return;
                }
            }

            MZNavigator.requestScreen(VPN.ScreenViewLogs);
        }
    }

    Connections {
        target: VPN
        function onAccountDeleted() {
            VPNController.logout();
        }
    }

    // Glean Connections
    Connections {
        target: VPN
        enabled: Qt.platform.os !== "android"

        function onInitializeGlean() {
            if (MZEnv.debugMode) {
                console.debug("Initializing glean with debug mode");
                Glean.setLogPings(true);
                // Uncomment for debugging purposes.
                // See: https://mozilla.github.io/glean/book/reference/debug/debugViewTag.html#debug-view-tag
                //
                // Glean.setDebugViewTag("MozillaVPN");
            }
            var channel = VPN.stagingMode ? "staging" : "production";

            console.debug("Initializing glean with channel set to:", channel);
            Glean.initialize("mozillavpn", MZSettings.gleanEnabled, {
                appBuild: "MozillaVPN/" + MZEnv.versionString,
                appDisplayVersion: MZEnv.versionString,
                channel: channel,
                osVersion: MZEnv.osVersion,
                architecture: [MZEnv.architecture, MZEnv.graphicsApi].join(" ").trim(),
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

        function onAboutToQuit() {
            console.debug("about to quit, shutdown Glean");
            // Submit the main ping in case there are outstading metrics in storage before shutdown.
            Pings.main.submit();
            // Use glean's built-in shutdown method - https://mozilla.github.io/glean/book/reference/general/shutdown.html
            Glean.shutdown();
        }
    }

    Connections {
        target: MZGleanDeprecated
        enabled: Qt.platform.os !== "android"

        function onRecordGleanEvent(sample) {
            console.debug("recording Glean event");
            Sample[sample].record();
        }

        function onRecordGleanEventWithExtraKeys(sample, extraKeys) {
            console.debug("recording Glean event with extra keys");
            Sample[sample].record(extraKeys);
        }
    }

    Connections {
        target: MZSettings
        function onGleanEnabledChanged() {
            console.debug("Glean - onGleanEnabledChanged", MZSettings.gleanEnabled);
            Glean.setUploadEnabled(MZSettings.gleanEnabled);
        }
    }

    MZTutorialPopups {
        id: tutorialUI
    }

    MZSystemAlert {
    }

    MZServerUnavailablePopup {
        id: serverUnavailablePopup
    }

    Connections {
        target: VPNController
        function onReadyToServerUnavailable(receivedPing) {
            serverUnavailablePopup.receivedPing = receivedPing
            serverUnavailablePopup.open();
        }
    }

    MZBottomNavigationBar {
        id: navbar

        property var showNavigationBar: [
            VPN.ScreenSettings,
            VPN.ScreenHome,
            VPN.ScreenMessaging,
            VPN.ScreenGetHelp,
            VPN.ScreenTipsAndTricks
        ]

        visible: showNavigationBar.includes(MZNavigator.screen) &&
                 VPN.userState === VPN.UserAuthenticated &&
                 VPN.state === VPN.StateMain && opacity !== 0

        function setNavBarOpacity() {
            if (MZNavigator.screen === VPN.ScreenHome) {
                navbar.opacity = VPNConnectionBenchmark.state === VPNConnectionBenchmark.StateInitial ? 1 : 0
            } else {
                navbar.opacity = 1;
            }
        }

        Connections {
            target: VPNConnectionBenchmark
            function onStateChanged() {
                navbar.setNavBarOpacity();
            }
        }

        Connections {
          target: MZNavigator

          function onCurrentComponentChanged() {
              navbar.setNavBarOpacity();
           }
        }

        Behavior on opacity {
            PropertyAnimation {
                duration: 500
            }
        }
    }

    Connections {
        target: VPNAppPermissions
        function onNotification(type,message,action) {
            console.log("Got notification: "+type + "  message:"+message);
            var component = Qt.createComponent("qrc:/nebula/components/MZAlert.qml");
            if(component.status !== Component.Ready)
                {
                    if( component.status == Component.Error )
                        console.debug("Error:"+ component.errorString() );

                }
            var alert = component.createObject(window, {
                                       isLayout:false,
                                       visible:true,
                                       alertText: message,
                                       alertType: type,
                                       alertActionText: action,
                                       duration:type === "warning"? 0: 2000,
                                       destructive:true,
                                       // Pin y height to be below the alert bar as we can't render above it
                                       setY: MZTheme.theme.windowMargin,
                                       onActionPressed: ()=>{VPNAppPermissions.openFilePicker();},
                                   });

            alert.show();
        }
    }
}
