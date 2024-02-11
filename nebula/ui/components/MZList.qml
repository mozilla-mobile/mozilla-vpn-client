/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls

import Mozilla.Shared 1.0

ListView {
    id: list

    property var accessibleName: ""

    height:contentHeight
    Accessible.role: Accessible.List
    Accessible.name: accessibleName
    Accessible.ignored: !visible
    boundsBehavior: Flickable.StopAtBounds
    highlightFollowsCurrentItem: true
    Keys.onDownPressed: list.incrementCurrentIndex()
    Keys.onUpPressed: list.decrementCurrentIndex()

    ScrollBar.vertical: ScrollBar {
        readonly property real scrollBarWidth: Qt.platform.os === "osx" ? 6 : 10
        id: scrollBar

        Accessible.ignored: true
        hoverEnabled: true
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: Qt.platform.os === "osx" ? 2 : 0

        background: Rectangle {
            color: MZTheme.theme.transparent
            anchors.fill: parent
        }

        contentItem: Rectangle {
            color: MZTheme.colors.grey40
            width: scrollBar.scrollBarWidth
            implicitWidth: scrollBar.scrollBarWidth
            radius: scrollBar.scrollBarWidth

            opacity: scrollBar.pressed ? 0.5 :
                   scrollBar.interactive && scrollBar.hovered ? 0.4 : 0.3
            Behavior on opacity {
                PropertyAnimation {
                    duration: 100
                }
            }
        }

        font.pixelSize: 0 // QTBUG-96733 workaround
        implicitWidth: scrollBarWidth
        width: scrollBarWidth
        minimumSize: 0

        visible: list.interactive

        Behavior on opacity {
            PropertyAnimation {
                duration: 100
            }
        }
    }

    // Scroll item into ListView's visible viewport region if necessary
    function ensureVisible(item) {
        if(ensureVisAnimation.running) {
            ensureVisAnimation.stop();
        }

        const itemY = item.mapToItem(list.contentItem, 0, 0).y;

        if (item.skipEnsureVisible || itemY < list.originY) {
            return;
        }

        const isItemBeyondNavBarTop = (item.mapToItem(window.contentItem, 0, 0).y + item.height) > (window.height - MZTheme.theme.navBarHeightWithMargins);
        const isItemBeyondWindowHeight = (item.mapToItem(window.contentItem, 0, 0).y + item.height) > window.height;
        const bottomMargin = (navbar.visible && isItemBeyondNavBarTop) ? MZTheme.theme.navBarHeightWithMargins : MZTheme.theme.contentBottomMargin;
        const itemHeight = item.height + bottomMargin;
        let scrollY;

        // Scroll the item upwards into view. Occurs if the item is beyond the navbar's top while navbar is visible. Or if the
        // item is beyond the window height while navbar is not visible.
        if((navbar.visible && isItemBeyondNavBarTop) || (!navbar.visible && isItemBeyondWindowHeight)) {
            const scrollDistance = item.mapToItem(window.contentItem, 0, 0).y + item.height - (window.height - bottomMargin);

            scrollY = list.contentY + scrollDistance;
        }
        else {
            // Scroll the item downwards into view
            if (itemY < list.contentY) {
                scrollY = itemY - bottomMargin;
            }
        }

        if (typeof(scrollY) === "undefined") {
            return;
        }

        // Animate scroll to destination
        ensureVisAnimation.to = scrollY;
        ensureVisAnimation.start();
    }

    NumberAnimation on contentY {
        id: ensureVisAnimation

        duration: 300
        easing.type: Easing.OutQuad
    }
}
