/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Rectangle {
    id: alertBox

    property var alertType: ""
    property var alertColor: Theme.redButton
    property var alertText: ""
    property var alertLinkText: ""

    color: "transparent"
    height: Math.max(40, (labelWrapper.height + Theme.windowMargin))
    width: parent.width - Theme.windowMargin
    y: parent.height - 48
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.margins: Theme.windowMargin / 2
    radius: Theme.cornerRadius

    VPNButtonBase {
        id: alertAction

        anchors.fill: alertBox
        radius: Theme.cornerRadius
        onClicked: {
            // TODO Try again
            // TODO Try again
            // TODO Restore

            switch (alertType) {
            case ("update"):
                stackview.push("../views/ViewUpdate.qml", StackView.Immediate);
                break;
            case ("authentication-failed"):
                VPN.authenticate();
                break;
            case ("connection-failed"):
            case ("no-connection"):
            case ("background-service"):
            default:
                VPN.hideAlert();
            }
        }

        VPNUIStates {
            itemToFocus: parent
            colorScheme: alertColor
            setMargins: -3
        }

        Rectangle {
            id: labelWrapper

            color: "transparent"
            height: label.paintedHeight
            anchors.left: alertAction.left
            width: alertAction.width - Theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            Label {
                id: label
                 anchors.centerIn: parent
                 text: alertBox.alertText + " " + "<b><u>" + alertLinkText + "</b></u>"
                 horizontalAlignment: Text.AlignHCenter
                 font.pixelSize: Theme.fontSizeSmall
                 color: Theme.white
                 width: labelWrapper.width - Theme.windowMargin
                 wrapMode: Label.WordWrap
             }

        }

        VPNMouseArea {
        }
    }

    VPNFocusOutline {
        focusColorScheme: alertColor
        focusedComponent: closeButton
        anchors.fill: closeButton
        setMargins: -3
    }

    VPNButtonBase {
        // Hack to create the two right angle corners
        // where closeButton meets alertAction

        id: closeButton

        height: parent.height
        width: Theme.rowHeight
        clip: true
        anchors.right: parent.right
        anchors.rightMargin: 0
        radius: 0
        Accessible.name: "Close"
        onClicked: {
            if (alertType === "update") {
                closeAlert.start();
                return VPN.hideUpdateRecommendedAlert();
            }
            return VPN.hideAlert();
        }

        VPNFocusBorder {
            anchors.fill: closeButton
            border.color: alertColor.focusBorder
            opacity: closeButton.activeFocus ? 1 : 0
            z: 1
        }

        Rectangle {
            id: backgroundRect

            height: parent.height
            width: parent.width + 10
            anchors.left: closeButton.left
            anchors.leftMargin: -10
            radius: 4
            color: "transparent"
            clip: true
            state: closeButton.state

            VPNUIStates {
                colorScheme: alertColor
                setMargins: -3
            }

            Behavior on color {
                ColorAnimation {
                    duration: 200
                }

            }

        }

        Image {
            id: alertBoxClose

            source: "../resources/close-white.svg"
            sourceSize.width: 12
            sourceSize.height: 12
            anchors.centerIn: closeButton
        }

        VPNMouseArea {
        }

    }

    VPNFocusBorder {
        anchors.fill: alertBox
        border.color: alertColor.focusBorder
        opacity: alertAction.activeFocus ? 1 : 0
    }

}
