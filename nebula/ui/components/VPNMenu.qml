/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

Item {
    id: menuBar

    property alias objectName: iconButton.objectName
    property alias title: title.text
    property alias rightTitle: rightTitle.text
    property bool accessibleIgnored: false
    property bool btnDisabled: false
    property alias forceFocus: iconButton.focus
    property string _iconButtonSource: "qrc:/nebula/resources/back.svg"
    property var _menuOnBackClicked: () => { goBack(); }

    width: parent.width
    height: VPNTheme.theme.menuHeight
    // Ensure that menu is on top of possible scrollable
    // content.
    z: 2

    Rectangle {
        id: menuBackground
        color: VPNTheme.theme.bgColor
        y: 0
        width: parent.width
        height: 55
    }

    VPNIconButton {
        id: iconButton

        skipEnsureVisible: true // prevents scrolling of lists when this is focused

        onClicked: _menuOnBackClicked()
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: VPNTheme.theme.windowMargin / 2
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        //% "Back"
        //: Go back
        accessibleName: qsTrId("vpn.main.back")
        Accessible.ignored: accessibleIgnored

        enabled: !btnDisabled
        opacity: enabled ? 1 : .4
        Image {
            objectName: "menuIcon"
            source: _iconButtonSource
            sourceSize.width: VPNTheme.theme.iconSize
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
        anchors.rightMargin: VPNTheme.theme.windowMargin
        Accessible.ignored: accessibleIgnored
    }

    Rectangle {
        color: VPNTheme.colors.grey10
        y: 55
        width: parent.width
        height: 1
    }

    function goBack() {
        if (typeof(stackview) !== "undefined") {
            stackview.pop();
        } else {
            mainStackView.pop();
        }
    }
}
