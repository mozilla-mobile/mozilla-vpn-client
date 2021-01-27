/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import "../themes/themes.js" as Theme

Item {
    id: menuBar

    property alias objectName: iconButton.objectName
    property alias title: title.text
    property alias rightTitle: rightTitle.text
    property bool isSettingsView: false
    property bool isMainView: false
    property bool accessibleIgnored: false
    property alias forceFocus: iconButton.focus

    width: parent.width
    height: 56
    // Ensure that menu is on top of possible scrollable
    // content.
    z: 1

    Rectangle {
        id: menuBackground
        color: Theme.bgColor
        y: 0
        width: parent.width
        height: 55
    }

    VPNIconButton {
        id: iconButton

        onClicked: isMainView ? mainStackView.pop() : (isSettingsView ? settingsStackView.pop() : stackview.pop())
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        //% "Back"
        //: Go back
        accessibleName: qsTrId("vpn.main.back")
        Accessible.ignored: accessibleIgnored

        Image {
            id: backImage

            source: "../resources/back.svg"
            sourceSize.width: Theme.iconSize
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }

    }

    VPNBoldLabel {
        id: title

        anchors.top: menuBar.top
        anchors.centerIn: menuBar
        Accessible.ignored: accessibleIgnored
    }

    VPNLightLabel {
        id: rightTitle

        anchors.verticalCenter: menuBar.verticalCenter
        anchors.right: menuBar.right
        anchors.rightMargin: Theme.windowMargin
        Accessible.ignored: accessibleIgnored
    }

    Rectangle {
        color: "#0C0C0D0A"
        y: 55
        width: parent.width
        height: 1
    }

}
