/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0
import compat 0.1

Popup {
    id: popup

    property int maxWidth: ({})

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    enabled: true
    focus: true
    modal: true
    width: Math.min(window.width - VPNTheme.theme.vSpacing, maxWidth)
    horizontalPadding: VPNTheme.theme.popupMargin

    background: Rectangle {
        id: popupBackground

        anchors.fill: contentItem
        anchors.margins: -24
        color: VPNTheme.theme.bgColor
        radius: 8

        Rectangle {
            id: popUpShadowSource
            anchors.fill: popupBackground
            radius: popupBackground.radius
            z: -1
        }

        VPNDropShadow {
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

        // Close button
        VPNIconButton {
            id: closeButton

            accessibleName: qsTrId("menubar.file.close")
            anchors {
                right: popupBackground.right
                top: popupBackground.top
                rightMargin: VPNTheme.theme.windowMargin / 2
                topMargin: VPNTheme.theme.windowMargin / 2
            }
            onClicked: {
                popup.close();
            }

            Image {
                id: closeImage

                anchors.centerIn: closeButton
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/close-darker.svg"
                sourceSize.height: VPNTheme.theme.iconSize
                sourceSize.width: VPNTheme.theme.iconSize
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
