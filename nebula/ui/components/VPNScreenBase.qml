/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import components 0.1
import telemetry 0.30


Item {
    id: root
    property alias _menuOnBackClicked: menu._menuOnBackClicked
    property alias _menuIconButtonSource: menu._menuIconButtonSource
    property alias _menuIconVisibility: menu._menuIconVisibility
    property var _onStackCompleted: () => {}

    Rectangle {
        anchors.fill: parent
        color: window.color
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        VPNMenu {
            id: menu
            objectName: root.objectName + "-back"

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: VPNTheme.theme.menuHeight

            _menuIconButtonSource: stackview.depth === 1 ? "qrc:/nebula/resources/close-dark.svg" : "qrc:/nebula/resources/back.svg"
            _menuIconButtonMirror:  stackview.depth !== 1 && VPNLocalizer.isRightToLeft
            _iconButtonAccessibleName: stackview.depth === 1 ? qsTrId("vpn.connectionInfo.close") : qsTrId("vpn.main.back")
            _menuOnBackClicked: () => maybeRequestPreviousScreen()
            titleComponent: stackview.currentItem.titleComponent ? stackview.currentItem.titleComponent : null
            rightButtonComponent: stackview.currentItem.rightMenuButton ? stackview.currentItem.rightMenuButton : null

            title: ""

            function maybeRequestPreviousScreen() {
                if (stackview.depth !== 1) {
                    return stackview.pop();
                }
                VPNNavigator.requestPreviousScreen();
            }
        }

        VPNStackView {
            id: stackview
            objectName: root.objectName + "-stackView"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            onCurrentItemChanged: {
                menu.title = Qt.binding(() => currentItem._menuTitle || "");
                menu._menuOnBackClicked = currentItem._menuOnBackClicked ? currentItem._menuOnBackClicked : () => menu.maybeRequestPreviousScreen()
            }

            Connections {
                target: menu
                function onRightMenuButtonClicked() {
                    menu.rightMenuButtonClicked()
                }
            }
        }
    }

    function getStack() {
        return stackview
    }
}
