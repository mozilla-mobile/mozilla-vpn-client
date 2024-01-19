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
              anchors. right: parent.right
              height: 20
              color: "green"

          }
      }
  }
*/

Loader {
    id: root

    readonly property int maxHeight: (Qt.platform.os === "ios" ? window.safeContentHeight : window.height) -  MZTheme.theme.sheetTopMargin
    required default property var contentItem

    signal close

    active: false

    onClose: item.close()

    sourceComponent: Drawer {
        implicitWidth: window.width
        implicitHeight: Math.min(contentItem.implicitHeight, maxHeight)

        topPadding: 0

        dragMargin: 0
        edge: Qt.BottomEdge
        contentItem: root.contentItem

        onClosed: root.active = false

        background: Rectangle {

            radius: 8
            color: MZTheme.theme.bgColor

            Rectangle {
                color: parent.color
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
            }
        }

        Overlay.modal: Rectangle {
            color: MZTheme.theme.overlayBackground
        }

        Component.onCompleted: console.log("Created XXX")
        Component.onDestruction: console.log("Destroyed XXX")
    }
}

