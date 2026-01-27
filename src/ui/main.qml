/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

ApplicationWindow {
    id: window

    signal showServerList
    signal screenClicked(double x, double y)
    signal unwindStackView

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
        // Notch (natch)
        case 1624: // iPhone_XR (Qt Provided Physical Resolution)
        case 1792: // iPhone_XR

        case 2436: // iPhone_X_XS
        case 2688: // iPhone_XS_MAX

        case 2532: // iPhone_12_Pro
        case 2778: // iPhone_12_Pro_Max
        case 2340: // iPhone_12_mini
            return 34;

        // Dynamic island
        case 2556: // iPhone_14_Pro, 15, 15 Pro, 16
        case 2622: // iPhone 16, 16 Pro, 17, 17 Pro
        case 2736: // iPhone Air
        case 2796: // iPhone_14_Pro_Max, 15 Plus, 15 Pro Max, 16 Plus
        case 2868: // iPhone 16 Pro Max, 17 Pro Max
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

    screen: Qt.platform.os === "wasm" && Qt.application.screens.length > 1 ? Qt.application.screens[1] : Qt.application.screens[0]
    flags: {
        var baseFlags = Qt.Window | Qt.NoTitlebarBackgroundHint;

        // Do not use edge-to-edge flags on Android API 28 and below
        // Older versions have issues with 3-button navigation overlapping content
        if (Qt.platform.os === "android" && (VPNAndroidCommons.getAndroidApiLevel() < 29)) {
            return baseFlags;
        }

        // workaround for Qt.ExpandedClientAreaHint misbehaving on windows
        // TODO: revise after Qt 6.10.2 is released
        if (Qt.platform.os === "windows") {
            return baseFlags;
        }

        return baseFlags | Qt.ExpandedClientAreaHint | Qt.MaximizeUsingFullscreenGeometryHint;
    }
    visible: true

    width: fullscreenRequired() ? Screen.width : MZTheme.theme.desktopAppWidth;
    height: fullscreenRequired() ? Screen.height : MZTheme.theme.desktopAppHeight;

    //These need to be bound before onComplete so that the window buttons, menus and title bar double click behave properly
    maximumWidth: fullscreenRequired() ? Screen.width : MZTheme.theme.desktopAppWidth;
    maximumHeight: fullscreenRequired() ? Screen.height : MZTheme.theme.desktopAppHeight;

    title: MZI18n.ProductName
    color: MZTheme.colors.bgColor
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

        color: MZTheme.colors.transparent
        height: safeAreaHeightByDevice()
        width: window.width
        anchors.top: parent.top
    }
    MZNavigatorLoader {
      id: screenLoader
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
                if(MZUtils.viewLogs()){
                    return;
                }
            }

            MZNavigator.requestScreen(VPN.ScreenViewLogs);
        }
    }

    Connections {
        target: VPN
        function onAccountDeleted() {
            VPN.logout();
        }
    }

    MZSystemAlert {
    }

    ServerUnavailablePopup {
        id: serverUnavailablePopup
    }

    Connections {
        target: VPNController
        function onReadyToServerUnavailable(receivedPing) {
            serverUnavailablePopup.receivedPing = receivedPing
            serverUnavailablePopup.open();
        }

        function onStateChanged() {
            if (VPNController.state !== VPNController.StateOn) {
                return
            }
            if (serverUnavailablePopup.opened) {
                serverUnavailablePopup.close();
                console.log("Connection stabilized and server unavailable popup visible. Closing server unavailable popup");
            }
        }
    }

    MZBottomNavigationBar {
        id: navbar

        property var showNavigationBar: [
            VPN.ScreenSettings,
            VPN.ScreenHome,
            VPN.ScreenMessaging,
            VPN.ScreenGetHelp,
        ]

        visible: showNavigationBar.includes(MZNavigator.screen) &&
                 VPN.state === VPN.StateMain
    }

    Shortcut {
        sequence: "Ctrl+M"
        enabled: Qt.platform.os === "osx"

        onActivated: {
            window.showMinimized();
        }
    }

    Shortcut {
        sequence: "Ctrl+,"
        enabled: Qt.platform.os === "osx"

        onActivated: {
            MZNavigator.requestScreen(VPN.ScreenSettings);
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
