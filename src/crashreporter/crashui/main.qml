/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Window 2.12
import Mozilla.VPN 1.0

import compat 0.1
import components 0.1

Window {
    id: window

    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height

    function fullscreenRequired() {
        return Qt.platform.os === "android" ||
                Qt.platform.os === "ios" ||
                Qt.platform.os === "tvos";
    }
    screen: Qt.platform.os === "wasm" && Qt.application.screens.length > 1 ? Qt.application.screens[1] : Qt.application.screens[0]
    flags: Qt.platform.os === "ios" ? Qt.MaximizeUsingFullscreenGeometryHint : Qt.Window
    visible: true

    width: fullscreenRequired() ? Screen.width : VPNTheme.theme.desktopAppWidth;
    height: fullscreenRequired() ? Screen.height : VPNTheme.theme.desktopAppHeight;

    //These need to be bound before onComplete so that the window buttons, menus and title bar double click behave properly
    maximumWidth: fullscreenRequired() ? Screen.width : VPNTheme.theme.desktopAppWidth;
    maximumHeight: fullscreenRequired() ? Screen.height : VPNTheme.theme.desktopAppHeight;

    //% "Mozilla Crash Reporter"
    title: qsTrId("vpn.crashreporter.mainTitle")
    color: "#F9F9FA"

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
    Column{
      anchors.centerIn: parent
      spacing: VPNTheme.theme.windowMargin
      anchors.margins: VPNTheme.theme.windowMargin
      Image {
          id: mainIcon
          source: "qrc:/crashresources/Warning.svg"
          anchors.left: parent.left
          anchors.right: parent.right
          horizontalAlignment: Image.AlignHCenter
          fillMode: Image.PreserveAspectFit
      }

      VPNSubtitle {
          id: mainHeading
          text: qsTrId("vpn.crashreporter.mainHeading")
          horizontalAlignment: Text.AlignHCenter
      }

      VPNTextBlock{
          id: description
          anchors.horizontalCenter: parent.horizontalCenter
          horizontalAlignment: Text.AlignHCenter
          text: qsTrId("vpn.crashreporter.description")
      }

      VPNButton {
        text: qsTrId("vpn.crashreporter.sendButtonLabel");
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: CrashController.sendReport()
      }
      VPNLinkButton {
        labelText: qsTrId("vpn.crashreporter.dontSendButton");
        anchors.horizontalCenter: parent.horizontalCenter
        onClicked: CrashController.userDecline()
      }
    }
    Component.onCompleted: {
        if (!fullscreenRequired()) {
            minimumHeight = VPNTheme.theme.desktopAppHeight
            minimumWidth = VPNTheme.theme.desktopAppWidth

        }
    }
}
