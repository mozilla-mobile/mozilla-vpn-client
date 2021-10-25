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

Item {
    id: root

    property alias _menuTitle: menu.title
    property alias isMainView: menu.isMainView
    property alias isSettingsView: menu.isSettingsView
    property alias licenseContent: licenseText.text
    property alias licenseTitle: menu.title

    VPNMenu {
        id: menu
    }

    VPNFlickable {
        id: license

        anchors {
            top: menu.bottom
            topMargin: Theme.listSpacing * 2
        }
        height: root.height - menu.height
        flickContentHeight: licenseText.height + Theme.windowMargin * 4
        width: parent.width

        VPNTextBlock {
            id: licenseText

            anchors.horizontalCenter: parent.horizontalCenter
            textFormat: Text.MarkdownText
            width: parent.width - Theme.windowMargin * 2

            onLinkActivated: link => VPN.openLinkUrl(link)
        }
    }

}
