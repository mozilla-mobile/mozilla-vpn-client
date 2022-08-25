/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0

VPNIconButton {
    id: root

    property alias source: image.source
    property bool hasNotification: true

    Layout.minimumWidth: VPNTheme.theme.rowHeight
    Layout.alignment: Qt.AlignHCenter
    Layout.preferredHeight: VPNTheme.theme.rowHeight

    accessibleName: ""
    backgroundRadius: height / 2
    buttonColorScheme: VPNTheme.theme.iconButtonDarkBackground
    uiStatesVisible: !checked

    onClicked: {
        checked = true
    }

    Rectangle {
        id: circleBackground
        anchors.fill: parent
        visible: checked
        radius: parent.height / 2
        opacity: .2
        color: VPNTheme.theme.white
    }

    Image {
        id: image
        anchors.centerIn: parent
        sourceSize.height: VPNTheme.theme.iconSize * 2
        sourceSize.width: VPNTheme.theme.iconSize * 2
    }
}
