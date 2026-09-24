/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1

MZFlickable {
    id: networkManagerUnavailable

    width: parent.width
    flickContentHeight: col.implicitHeight + col.anchors.topMargin

    ColumnLayout {
        id: col

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: MZTheme.theme.vSpacingSmall

        MZHeadline {
            id: headline

            text: MZI18n.NetworkManagerUnavailableTitle
            horizontalAlignment: Text.AlignLeft

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.contentTopMarginDesktop
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        }

        MZInterLabel {
            id: body

            text: MZI18n.NetworkManagerUnavailableBody
            horizontalAlignment: Text.AlignLeft

            Layout.topMargin: 8
            Layout.fillWidth: true
            Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
            Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        }

        Item {
            Layout.fillHeight: !window.fullscreenRequired()
        }

        MZSignOut {
            id: signOff

            Layout.preferredHeight: MZTheme.theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            anchors.horizontalCenter: undefined
            anchors.bottom: undefined
            anchors.bottomMargin: undefined
            height: undefined
        }
    }
}
