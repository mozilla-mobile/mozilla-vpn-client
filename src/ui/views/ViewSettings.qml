/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    VPNMenu {
        id: menu
        objectName: "settingsBackButton"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        title: ""
        isSettingsView: true
        visible: settingsStackView.depth !== 1
        opacity: visible ? 1 : 0


        Behavior on opacity {
            PropertyAnimation {
                duration: 200
            }
        }
    }


    VPNStackView {
        id: settingsStackView
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Component.onCompleted: {
            settingsStackView.push("qrc:/ui/settings/ViewSettingsMenu.qml")
        }

        onCurrentItemChanged: {
            menu.title = Qt.binding(() => currentItem._menuTitle || "");
            menu.visible = Qt.binding(() => currentItem._menuVisible !== undefined
                ? currentItem._menuVisible
                : settingsStackView.depth !== 1);
        }
    }
}
