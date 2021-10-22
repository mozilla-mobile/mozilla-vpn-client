/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import themes 0.1

VPNFlickable {
    id: license

    property alias isSettingsView: menu.isSettingsView
    property alias isMainView: menu.isMainView
    property alias licenseTitle: menu.title
    property alias licenseContent: licenseText.text
    property alias _menuTitle: menu.title

    flickContentHeight: menu.height + licenseText.height + (Theme.windowMargin * 4)

    VPNMenu {
        id: menu
    }

    VPNTextBlock {
        id: licenseText

        width: parent.width
        textFormat: Text.MarkdownText

        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: menu.bottom
        anchors.leftMargin: Theme.windowMargin
        anchors.rightMargin: Theme.windowMargin

        onLinkActivated: link => VPN.openLinkUrl(link)
    }
}
