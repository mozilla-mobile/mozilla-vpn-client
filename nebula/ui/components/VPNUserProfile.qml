/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


VPNClickableRow {
    id: userInfo
    canGrowVertical: true

    property string _objNameBase: "settingsUserProfile"
    property string _iconSource: ""
    property alias _iconMirror: icon.mirror
    property bool _loaderVisible: false
    property var _buttonOnClicked


    Layout.preferredHeight: row.implicitHeight + VPNTheme.theme.windowMargin * 2
    Layout.fillWidth: true
    Layout.leftMargin: VPNTheme.theme.windowMargin / 2
    Layout.rightMargin: VPNTheme.theme.windowMargin / 2
    accessibleName: qsTrId("vpn.main.manageAccount")
    objectName: _objNameBase + "-manageAccountButton"
    onClicked: _buttonOnClicked()
    height: undefined
    anchors {
        left: undefined
        right: undefined
    }

    anchors {
        left: undefined
        right: undefined
        leftMargin: undefined
        rightMargin: undefined
    }

    RowLayout {
        id: row
        spacing: VPNTheme.theme.windowMargin
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: VPNTheme.theme.windowMargin / 2
        width: parent.width - anchors.leftMargin

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
                objectName: _objNameBase + "-displayName"
                readonly property var textVpnUser: VPNI18n.GlobalVpnUser
                text: VPNUser.displayName ? VPNUser.displayName : textVpnUser
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                Layout.fillWidth: true
            }

            VPNTextBlock {
                id: serverLocation
                objectName: _objNameBase + "-emailAddress"
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

        Rectangle {
            id: iconButton
            objectName: _objNameBase + "-manageAccountButton"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.preferredWidth: VPNTheme.theme.rowHeight
            visible: _iconSource !== ""
            color: VPNTheme.theme.transparent

            VPNIcon {
                id: icon
                source: _iconSource
                fillMode: Image.PreserveAspectFit
                anchors.centerIn: iconButton
                visible: !_loaderVisible
            }

            VPNButtonLoader {
                id: loader

                color: VPNTheme.theme.transparent
                iconUrl: "qrc:/nebula/resources/spinner.svg"
                state: _loaderVisible ? "active" : "inactive"
            }
        }
    }
}
