/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import "../themes/themes.js" as Theme
Item {
    property alias logo: logo.source
    property alias logoTitle: logoTitle.text
    property alias logoSubtitle: logoSubtitle.text
    property var logoSize: 76
    property var maskImage: false
    property var isSettingsView: false

    anchors.horizontalCenter: parent.horizontalCenter
    width: Math.min(parent.width, Theme.maxHorizontalContentWidth)
    height: panel.implicitHeight

    ColumnLayout {
        id: panel

        anchors.leftMargin: Theme.windowMargin * 1.5
        anchors.rightMargin: Theme.windowMargin * 1.5
        width: parent.width - Theme.windowMargin * 3
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0

        Item {
            id: logoWrapper

            Layout.preferredHeight: Math.max(logoSize, 76)
            Layout.fillWidth: true

            Image {
                id: logo

                visible: !isSettingsView
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: logoWrapper.bottom
                verticalAlignment: Image.AlignBottom
                anchors.bottomMargin: 0
                sourceSize.height: logoSize
                sourceSize.width: logoSize
                fillMode: Image.PreserveAspectFit
                layer.enabled: true

                Rectangle {
                    id: mask

                    anchors.fill: parent
                    radius: logoSize / 2
                    visible: false
                }
            }

            VPNAvatar {
                id: avatar

                visible: isSettingsView
                avatarUrl: logo.source
                anchors.fill: logoWrapper
            }
        }

        VPNHeadline {
            id: logoTitle

            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 24
            // In Settings, the headline wrapMode is set to 'WrapAtWordBoundaryOrAnywhere' to
            // prevent very long, unbroken display names from throwing the layout
            wrapMode: isSettingsView ? Text.WrapAtWordBoundaryOrAnywhere : Text.WordWrap
        }

        VPNSubtitle {
            id: logoSubtitle

            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 12
            Layout.leftMargin: Theme.windowMargin / 2
            Layout.rightMargin: Theme.windowMargin / 2
            Layout.maximumWidth: Theme.maxHorizontalContentWidth
            Layout.fillWidth: true
        }

    }

}
