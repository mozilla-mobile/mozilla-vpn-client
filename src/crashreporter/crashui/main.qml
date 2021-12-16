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

Window {
    id: window

    property var safeContentHeight: window.height - iosSafeAreaTopMargin.height

    function fullscreenRequired() {
        return Qt.platform.os === "android" ||
                Qt.platform.os === "ios" ||
                Qt.platform.os === "tvos";
    }

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

}
