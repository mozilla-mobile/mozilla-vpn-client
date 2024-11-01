/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import components 0.1
import Mozilla.Shared 1.0

//SUMMARY: MZBottomSheet opens a drawer from the bottom of the screen showing it's contentItem

//USAGE:
/*
  MZBottomSheet {
      contentItem: ColumnLayout {
          Rectangle {
              anchors.left: parent.left
              anchors.right: parent.right
              height: 20
              color: MZTheme.colors.green90

          }
      }
  }
*/

Loader {
    id: root

    readonly property int maxSheetHeight: (Qt.platform.os === "ios" ? window.safeContentHeight : window.height) -  MZTheme.theme.sheetTopMargin
    required default property var contentItem
    property bool sizeToContent: false
    property bool isOpen: active ? item.opened : null

    signal opened
    signal closed

    function open() {
        active = true
        item.open()
    }

    function close() {
        item.close()
    }

    active: false

    sourceComponent: Drawer {
        implicitWidth: window.width
        implicitHeight: root.sizeToContent ? Math.min(contentItem.implicitHeight, maxSheetHeight) : maxSheetHeight

        topPadding: 0

        dragMargin: 0
        edge: Qt.BottomEdge
        contentItem: root.contentItem

        onOpened:  root.opened()

        onClosed: {
            root.closed()
            root.active = false
        }

        background: Rectangle {

            radius: 8
            color: MZTheme.colors.bgColor

            Rectangle {
                color: parent.color
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
            }
        }

        Overlay.modal: Rectangle {
            color: MZTheme.colors.overlayBackground
        }
    }
}

