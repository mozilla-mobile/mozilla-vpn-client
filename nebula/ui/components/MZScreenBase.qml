/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1

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

        MZMenu {
            id: menu
            objectName: root.objectName + "-back"

            Layout.preferredWidth: parent.width
            Layout.preferredHeight: MZTheme.theme.menuHeight

            _menuIconButtonSource: stackview.depth === 1 ? "qrc:/nebula/resources/close-dark.svg" : "qrc:/nebula/resources/back.svg"
            _menuIconButtonMirror:  stackview.depth !== 1 && MZLocalizer.isRightToLeft
            _iconButtonAccessibleName: stackview.depth === 1 ? MZI18n.GlobalClose : MZI18n.GlobalGoBack
            _menuOnBackClicked: () => maybeRequestPreviousScreen()
            titleComponent: stackview.currentItem && stackview.currentItem.titleComponent
                ? stackview.currentItem.titleComponent : null
            rightButtonComponent: stackview.currentItem && stackview.currentItem.rightMenuButton
                ? stackview.currentItem.rightMenuButton : null

            title: ""

            function maybeRequestPreviousScreen() {
                if (stackview.depth !== 1) {
                    return stackview.pop();
                }
                MZNavigator.requestPreviousScreen();
            }
        }

        MZStackView {
            id: stackview
            objectName: root.objectName + "-stackView"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            onCurrentItemChanged: {
                menu.title = Qt.binding(() => currentItem._menuTitle || "");
                menu._menuOnBackClicked = currentItem._menuOnBackClicked ? currentItem._menuOnBackClicked : () => menu.maybeRequestPreviousScreen();
                currentItem.forceActiveFocus();
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
