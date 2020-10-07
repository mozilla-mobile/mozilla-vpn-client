/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11

import "../themes/themes.js" as Theme

RoundButton {
    id: button

    height: 40
    width: Theme.maxTextWidth
    anchors.horizontalCenter: parent.horizontalCenter

    Layout.preferredHeight: 40
    Layout.fillWidth: true

    Layout.leftMargin: 16
    Layout.rightMargin: 16

    background: Rectangle {
        color: "#0060DF"
        radius: 4
    }

    contentItem: Label {
        id: label
        color: "#FFFFFF"
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.family: vpnFont.name
        font.pixelSize: 15
        font.weight: Font.Bold
    }
}
