/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


RowLayout {
    property string _iconButtonImageSource: ""
    property bool _loaderVisible: false
    property var _iconButtonOnClicked

    id: userInfo
    spacing: VPNTheme.theme.windowMargin

    Rectangle {
        Layout.preferredWidth: VPNTheme.theme.rowHeight
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.alignment: Qt.AlignVCenter
        color: VPNTheme.theme.transparent

        VPNAvatar {
            id: avatar
            avatarUrl: VPNUser.avatar
            anchors.fill: parent
        }
    }

    ColumnLayout {
        spacing: VPNTheme.theme.listSpacing

        Layout.alignment: Qt.AlignVCenter

        VPNBoldLabel {
            objectName: userInfo.objectName + "-displayName"
            readonly property var textVpnUser: VPNl18n.GlobalVpnUser
            text: VPNUser.displayName ? VPNUser.displayName : textVpnUser
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            Layout.fillWidth: true
        }

        VPNTextBlock {
            id: serverLocation
            objectName: userInfo.objectName + "-emailAddress"
            text: VPNUser.email
            Accessible.ignored: true
            Layout.alignment: Qt.AlignLeft
            width: undefined
            Layout.fillWidth: true
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            lineHeight: 1
            lineHeightMode: Text.FixedHeight
        }
    }

    VPNIconButton {
        id: iconButton
        objectName: userInfo.objectName + "-manageAccountButton"
        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        Layout.preferredHeight: VPNTheme.theme.rowHeight
        Layout.preferredWidth: VPNTheme.theme.rowHeight
        accessibleName: qsTrId("vpn.main.manageAccount")
        onClicked: _iconButtonOnClicked()
        visible: _iconButtonImageSource !== ""

        VPNIcon {
            id: icon
            source: _iconButtonImageSource
            fillMode: Image.PreserveAspectFit
            anchors.centerIn: iconButton
        }

        VPNButtonLoader {
            id: loader

            color: VPNTheme.theme.bgColor
            iconUrl: "qrc:/nebula/resources/spinner.svg"
            state: _loaderVisible ? "active" : "inactive"
        }
    }
}
