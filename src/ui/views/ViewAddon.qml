/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    property var addon
    property string _menuTitle: "Addon:" + addon.name

    id: addonWrapper

    Rectangle {
        anchors.fill: addonWrapper
        color: window.color
    }

    VPNMenu {
        id: menu

        title: "Addon:" + parent.addon.name
        anchors.top: parent.top
    }

    Loader {
        asynchronous: true

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.top: menu.bottom

        source: "qrc" + parent.addon.qml
    }

    Connections {
        target: VPNAddonManager
        function onUnloadAddon(addonId) {
            if (addonId === addon.id) {
                return mainStackView.pop(addonWrapper, StackView.Immediate);
            }
        }
    }
}
