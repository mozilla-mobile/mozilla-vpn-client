/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import "../themes/themes.js" as Theme

Item {
    property alias title: title.text
    property alias rightTitle: rightTitle.text
    property bool isSettingsView: false

    id: menuBar
    width: parent.width
    height: 56

    VPNIconButton {
        id: iconButton
        onClicked: isSettingsView ? settingsStackView.pop() : stackview.pop()

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: Theme.windowMargin / 2
        anchors.leftMargin: Theme.windowMargin / 2
        //% "Back"
        //: Go back 
        accessibleName: qsTrId("back")

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
    }

    VPNLightLabel {
        id: rightTitle
        anchors.verticalCenter: menuBar.verticalCenter
        anchors.right: menuBar.right
        anchors.rightMargin: Theme.windowMargin
    }

    Rectangle {
        color: "#0C0C0D0A"
        y: 55
        width: parent.width
        height: 1
    }
}
