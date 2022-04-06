/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Popup {
    id: popup

    property int maxWidth: ({})
    property alias _popupContent: popupContent.data

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    enabled: true
    focus: true
    modal: true
    width: Math.min(window.width - VPNTheme.theme.vSpacing, maxWidth)
    horizontalPadding: VPNTheme.theme.popupMargin

    contentItem: ColumnLayout {
        spacing: 0
        // Close button
        VPNIconButton {
            id: closeButton

            accessibleName: qsTrId("menubar.file.close")
            onClicked: {
                popup.close();
            }

            Layout.preferredHeight: VPNTheme.theme.rowHeight
            Layout.preferredWidth: VPNTheme.theme.rowHeight
            Layout.margins: VPNTheme.theme.windowMargin / 2
            Layout.alignment: Qt.AlignRight

            Image {
                id: closeImage
                anchors.centerIn: closeButton
                fillMode: Image.PreserveAspectFit
                source: "qrc:/nebula/resources/close-darker.svg"
                sourceSize.height: VPNTheme.theme.iconSize
                sourceSize.width: VPNTheme.theme.iconSize
            }
        }

        ColumnLayout {
            id: popupContent
            Layout.leftMargin: VPNTheme.theme.vSpacing
            Layout.rightMargin: VPNTheme.theme.vSpacing
            Layout.bottomMargin: VPNTheme.theme.vSpacing
            Layout.alignment: Qt.AlignHCenter
        }
    }

    background: Rectangle {
        id: popupBackground

        anchors.fill: contentItem
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

        color: VPNTheme.theme.overlayBackground

        Behavior on opacity {
            NumberAnimation {
                duration: 175
            }
        }
    }
}
