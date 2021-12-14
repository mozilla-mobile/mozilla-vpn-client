/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

VPNTextField {
    // TODO Add strings for Accessible.description, Accessible.name
    property bool hasError: false

    id: searchBar

    background: VPNInputBackground {}
    leftInset: 48
    leftPadding: 48
    onActiveFocusChanged: if (focus && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(searchBar)

    VPNIcon {
        source: "qrc:/nebula/resources/search.svg"
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 20
        sourceSize.height: VPNTheme.theme.windowMargin
        sourceSize.width: VPNTheme.theme.windowMargin
        opacity: parent.focus ? 1 : 0.8
    }

    RowLayout {
        id: searchWarning
        anchors.top: searchBar.bottom
        anchors.topMargin: VPNTheme.theme.listSpacing
        visible: searchBar.hasError
        width: parent.width
        spacing: VPNTheme.theme.windowMargin / 2

        VPNIcon {
            id: warningIcon

            source: "qrc:/nebula/resources/warning.svg"
            sourceSize.height: 14
            sourceSize.width: 14
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: VPNTheme.theme.windowMargin / 4
        }

        VPNInterLabel {
            id: warningLabel
            color: Color.error.default
            text: VPNl18n.ServersViewSearchNoResultsLabel
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            width: undefined
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }
    }

    Keys.onPressed: event => {
        if (focus && hasError && (/[\w\[\]`!@#$%\^&*()={}:;<>+'-]/).test(event.text)) {
            event.accepted = true;
        }
    }
}
