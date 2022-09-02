/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

Item {
    id: menuBar

    property alias objectName: iconButton.objectName
    property alias title: title.text
    property alias rightTitle: rightTitle.text
    property bool accessibleIgnored: false
    property bool btnDisabled: false
    property alias forceFocus: iconButton.focus
    property string _menuIconButtonSource: "qrc:/nebula/resources/back.svg"
    property alias _iconButtonAccessibleName: iconButton.accessibleName
    property var _menuOnBackClicked: () => {}
    property alias _menuIconVisibility: iconButton.visible
    property Component titleComponent
    property Component rightButtonComponent

    signal rightMenuButtonClicked()

    width: parent.width
    height: VPNTheme.theme.menuHeight
    // Ensure that menu is on top of possible scrollable
    // content.
    z: 2
    visible: title.text !== "" || titleComponent !== null || rightButtonComponent !== null

    Rectangle {
        id: menuBackground
        color: VPNTheme.theme.bgColor
        anchors.fill: parent
    }

    RowLayout {
        id: row

        objectName: "menuBar"
        anchors {
            fill: parent
            margins: VPNTheme.theme.windowMargin / 2
        }
        spacing: VPNTheme.theme.windowMargin / 4

        VPNIconButton {
            id: iconButton

            skipEnsureVisible: true // prevents scrolling of lists when this is focused

            onClicked: _menuOnBackClicked()
            Layout.alignment: Qt.AlignLeft

            accessibleName: _menuIconButtonSource.includes("close") ? qsTrId("vpn.connectionInfo.close") : qsTrId("vpn.main.back")
            Accessible.ignored: accessibleIgnored
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.preferredWidth: VPNTheme.theme.rowHeight
            enabled: !btnDisabled
            opacity: enabled ? 1 : .4

            Image {
                objectName: "menuIcon"
                source: _menuIconButtonSource
                sourceSize.width: VPNTheme.theme.iconSize
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: iconButton
            }
        }

        // This is a hack to preven the menu title from being thrown off horizontal center by varying 'rightTitle' widths.
        Rectangle {
            Layout.preferredWidth: rightTitle.width > VPNTheme.theme.rowHeight ? Math.max(rightTitle.width - VPNTheme.theme.rowHeight - row.spacing, 0) : 0
            Layout.preferredHeight: parent.height
            color: VPNTheme.theme.transparent
        }

        VPNBoldLabel {
            id: title

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            visible: text !== ""
            elide: Text.ElideRight
            Accessible.ignored: accessibleIgnored
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        VPNLightLabel {
            id: rightTitle

            visible: text !== ""
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.minimumWidth: VPNTheme.theme.rowHeight
            Layout.maximumWidth: row.width / 3
            elide: Text.ElideRight
            Accessible.ignored: accessibleIgnored
            rightPadding: VPNTheme.theme.windowMargin / 2
            horizontalAlignment: Text.AlignRight
        }
    }

    Loader {
        id: titleLoader

        anchors.centerIn: parent

        sourceComponent: menuBar.titleComponent
    }

    Loader {
        id: rightMenuButtonLoader

        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: VPNTheme.theme.windowMargin

        sourceComponent: menuBar.rightButtonComponent
    }

    Rectangle {
        color: VPNTheme.colors.grey10
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
    }
}
