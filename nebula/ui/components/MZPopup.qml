/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.Shared 1.0
import compat 0.1

Popup {
    id: popup

    default property alias _popupContent: popupContent.data
    property alias popupContentItem: popupContent //this is exposed so popupContent implicitHeight can (and should) be set externally
    property bool showCloseButton: true
    property bool startContentBeneathCloseButton: showCloseButton //popup content will appear under the close button
    property string closeButtonObjectName: ""

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    focus: true
    modal: true
    width: Math.min(window.width - (MZTheme.theme.windowMargin * 2), 500)
    contentHeight: startContentBeneathCloseButton ? closeButton.height + closeButton.anchors.topMargin + popupContent.implicitHeight + popupContent.anchors.topMargin: popupContent.implicitHeight
    horizontalPadding: 0
    verticalPadding: 0

    // Close button
    MZIconButton {
        id: closeButton

        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: MZTheme.theme.windowMargin / 2
        anchors.rightMargin: MZTheme.theme.windowMargin / 2

        height: MZTheme.theme.rowHeight
        width: MZTheme.theme.rowHeight
        visible: showCloseButton

        objectName: closeButtonObjectName
        accessibleName: MZI18n.GlobalClose

        onClicked: {
            popup.close();
        }

        Image {
            anchors.centerIn: closeButton

            fillMode: Image.PreserveAspectFit
            source: "qrc:/nebula/resources/close-darker.svg"
            sourceSize.height: MZTheme.theme.iconSize
            sourceSize.width: MZTheme.theme.iconSize
        }
    }

    Item {
        id: popupContent

        anchors.top: startContentBeneathCloseButton ? closeButton.bottom : undefined
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: startContentBeneathCloseButton ? MZTheme.theme.windowMargin / 2 : 0
    }

    background: Rectangle {
        id: popupBackground

        anchors.fill: parent
        color: MZTheme.theme.bgColor
        radius: 8

        Rectangle {
            id: popUpShadowSource
            anchors.fill: popupBackground
            radius: popupBackground.radius
            z: -1
        }

        MZDropShadow {
            id: popupShadow

            anchors.fill: popUpShadowSource
            cached: true
            color: "black"
            opacity: 0.2
            radius: 16
            source: popUpShadowSource
            spread: 0.1
            transparentBorder: true
            verticalOffset: 4
            z: -1
        }
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 0.0
            to: 1.0
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 120
            from: 1.0
            to: 0.0
            easing.type: Easing.InOutQuad
        }
    }

    Overlay.modal: Rectangle {
        id: overlayBackground

        color: MZTheme.theme.overlayBackground

        Behavior on opacity {
            NumberAnimation {
                duration: 175
            }
        }
    }
}
