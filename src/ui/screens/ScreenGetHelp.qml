/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    property int unlockCounter: 0

    Rectangle {
        anchors.fill: parent
        color: window.color
    }

    VPNMenu {
        id: menu
        objectName: "getHelpBack"
        _menuOnBackClicked: () => VPNNavigator.requestPreviousScreen()
        _menuIconButtonSource: getHelpStackView.depth === 1 ? "qrc:/nebula/resources/close-dark.svg" : "qrc:/nebula/resources/back.svg"
    }

    VPNMouseArea {
        anchors.fill: menu
        hoverEnabled: getHelpStackView.depth === 1
        cursorShape: Qt.ArrowCursor
        onMouseAreaClicked: function() {
            if (unlockCounter >= 5) {
                unlockCounter = 0
                VPNSettings.developerUnlock = true
            }
            else if (!VPNSettings.developerUnlock) {
                unlockTimeout.restart()
                unlockCounter = unlockCounter + 1
            }
        }
    }

    Timer {
        id: unlockTimeout
        repeat: false
        running: false
        interval: 10000
        onTriggered: unlockCounter = 0
    }

    VPNStackView {
        id: getHelpStackView

        anchors {
            top: menu.buttom
        }

        Component.onCompleted: function() {
            VPNNavigator.addStackView(VPNNavigator.ScreenGetHelp, getHelpStackView)
            getHelpStackView.push("qrc:/ui/screens/getHelp/ViewGetHelp.qml")
        }

        onCurrentItemChanged: {
            menu.title = Qt.binding(() => currentItem._menuTitle || "");
            menu.visible = Qt.binding(() => menu.title !== "");
            menu._menuOnBackClicked = currentItem._menuOnBackClicked ? currentItem._menuOnBackClicked : () => getHelpStackView.pop()
        }
    }
}
