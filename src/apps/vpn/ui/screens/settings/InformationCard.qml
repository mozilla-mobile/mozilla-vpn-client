/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.12

import Mozilla.VPN 1.0
import compat 0.1
import components 0.1

import org.mozilla.Glean 0.30
import telemetry 0.30

Rectangle {
    property alias _infoContent: infoContent.data
    id: card

    color: VPNTheme.theme.white
    radius: 4

    VPNDropShadow {
        anchors.fill: dropShadowSource
        source: dropShadowSource

        horizontalOffset: 1
        verticalOffset: 1
        radius: 6
        color: VPNTheme.colors.grey60
        opacity: .15
        transparentBorder: true
        cached: true
    }

    Rectangle {
        id: dropShadowSource
        anchors.fill: card

        color: VPNTheme.theme.white
        radius: card.radius
    }

    RowLayout {
        id: info
        spacing: VPNTheme.theme.windowMargin * 0.75

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: VPNTheme.theme.windowMargin
            rightMargin: VPNTheme.theme.windowMargin
            verticalCenter: parent.verticalCenter
        }

        VPNIcon {
            source: "qrc:/nebula/resources/info.svg"
            Layout.alignment: Qt.AlignTop
        }

        ColumnLayout {
            id: infoContent
            Layout.fillWidth: true
            spacing: 0
        }
    }
}

