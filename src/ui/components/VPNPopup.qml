/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import "../themes/themes.js" as Theme

Popup {
    id: popup

    property int maxWidth: ({})

    closePolicy: Popup.CloseOnEscape
    enabled: true
    focus: true
    modal: true
    implicitWidth: parent.width
    leftInset: maxWidth && maxWidth < window.width ? (window.width - maxWidth) / 2 : Theme.windowMargin
    rightInset: maxWidth && maxWidth < window.width ? (window.width - maxWidth) / 2 : Theme.windowMargin
    horizontalPadding: Theme.popupMargin + popup.leftInset
    bottomPadding: Theme.popupMargin
    topPadding: Theme.popupMargin * 2

    background: Rectangle {
        id: popupBackground

        anchors.margins: 0
        color: Theme.bgColor
        radius: 8

        Rectangle {
            id: popUpShadowSource
            anchors.fill: popupBackground
            radius: popupBackground.radius
            z: -1
        }

        DropShadow {
            id: popupShadow

            anchors.fill: popUpShadowSource
            cached: true
            color: "black"
            opacity: 0.2
            radius: 16
            samples: 33
            source: popUpShadowSource
            spread: 0.1
            transparentBorder: true
            verticalOffset: 4
            z: -1
        }

        // Close button
        VPNIconButton {
            id: closeButton

            accessibleName: qsTrId("menubar.file.close")
            anchors {
                right: popupBackground.right
                top: popupBackground.top
                rightMargin: Theme.windowMargin / 2
                topMargin: Theme.windowMargin / 2
            }
            onClicked: {
                popup.close();
            }

            Image {
                id: closeImage

                anchors.centerIn: closeButton
                fillMode: Image.PreserveAspectFit
                source: "../resources/close-darker.svg"
                sourceSize.height: Theme.iconSize
                sourceSize.width: Theme.iconSize
            }
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

        color: "#4D000000"

        Behavior on opacity {
            NumberAnimation {
                duration: 175
            }
        }
    }
}
