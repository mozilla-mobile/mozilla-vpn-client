/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0

Item {
    id: menuBar

    property alias objectName: iconButton.objectName
    property alias title: title.text
    property bool accessibleIgnored: false
    property bool btnDisabled: false
    property alias forceFocus: iconButton.focus
    property string _menuIconButtonSource: "qrc:/nebula/resources/back.svg"
    property alias _menuIconButtonMirror: menuIcon.mirror
    property alias _iconButtonAccessibleName: iconButton.accessibleName
    property var _menuOnBackClicked: () => {}
    property alias _menuIconVisibility: iconButton.visible
    property var titleClicked: () => {}
    property Component titleComponent
    property Component rightButtonComponent

    signal rightMenuButtonClicked()

    width: parent.width
    height: MZTheme.theme.menuHeight
    // Ensure that menu is on top of possible scrollable
    // content.
    z: 2
    visible: title.text !== "" || titleComponent !== null || rightButtonComponent !== null

    Rectangle {
        id: menuBackground
        color: MZTheme.theme.bgColor
        anchors.fill: parent
    }

    MZIconButton {
        id: iconButton

        skipEnsureVisible: true // prevents scrolling of lists when this is focused

        onClicked: _menuOnBackClicked()

        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: MZTheme.theme.windowMargin / 2

        accessibleName: _menuIconButtonSource.includes("close") ? qsTrId("vpn.connectionInfo.close") : MZI18n.GlobalGoBack
        Accessible.ignored: accessibleIgnored
        height: MZTheme.theme.rowHeight
        width: MZTheme.theme.rowHeight
        enabled: !btnDisabled
        opacity: enabled ? 1 : .4

        Image {
            id: menuIcon
            source: _menuIconButtonSource
            mirror: MZLocalizer.isRightToLeft
            sourceSize.width: MZTheme.theme.iconSize
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }
    }

    MZBoldLabel {
        id: title

        anchors.centerIn: parent
        width: getTitleWidth()

        visible: text !== ""
        elide: Text.ElideRight
        Accessible.ignored: accessibleIgnored
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
            onClicked: titleClicked()
        }
    }

    Loader {
        id: titleLoader

        anchors.centerIn: parent
        width: getTitleWidth()

        sourceComponent: menuBar.titleComponent
    }

    Loader {
        id: rightMenuButtonLoader

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: MZTheme.theme.windowMargin

        sourceComponent: menuBar.rightButtonComponent
        onItemChanged: {
            if (item instanceof Text) {
                anchors.rightMargin = MZTheme.theme.windowMargin
            }
            else if(item instanceof MZIconButton) {
                anchors.rightMargin = MZTheme.theme.windowMargin / 2
            }
        }
    }

    Rectangle {
        color: MZTheme.colors.grey10
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
    }

    function getTitleWidth() {
        return Math.min(parent.width - iconButton.width - rightMenuButtonLoader.width - iconButton.anchors.leftMargin - rightMenuButtonLoader.anchors.rightMargin,
                        parent.width - (Math.max(rightMenuButtonLoader.width + rightMenuButtonLoader.anchors.rightMargin, iconButton.width + iconButton.anchors.leftMargin)) * 2 - 8)
    }
}
