/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

Item {
    Rectangle {
        anchors.fill: parent
        color: window.color
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        MZMenu {
            id: menu

            property int unlockCounter: 0

            objectName: "getHelpBack"

            Layout.fillWidth: true
            
            _menuOnBackClicked: () => VPNNavigator.requestPreviousScreen()
            _menuIconButtonSource: getHelpStackView.depth === 1 ? "qrc:/nebula/resources/close-dark.svg" : "qrc:/nebula/resources/back.svg"
            _menuIconButtonMirror: getHelpStackView.depth !== 1 && MZLocalizer.isRightToLeft
            titleClicked: () => {
                            if (unlockCounter >= 5) {
                                unlockCounter = 0
                                MZSettings.developerUnlock = true
                            }
                            else if (!MZSettings.developerUnlock) {
                                unlockTimeout.restart()
                                unlockCounter = unlockCounter + 1
                            }
                        }

            Timer {
                id: unlockTimeout
                repeat: false
                running: false
                interval: 10000
                onTriggered: menu.unlockCounter = 0
            }
        }

        MZStackView {
            id: getHelpStackView
            objectName: "getHelpStackView"

            Layout.fillWidth: true
            Layout.fillHeight: true

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
}
