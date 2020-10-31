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

    color: alertColor.defaultColor
    height: Math.max(40, (label.paintedHeight + Theme.windowMargin))
    width: parent.width - Theme.windowMargin
    y: parent.height - 48
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.margins: Theme.windowMargin / 2
    radius: 4

    VPNButtonBase {
        id: alertAction

        bgColor: alertColor
        enableHover: alertLinkText.length > 0
        targetEl: alertBox
        anchors.verticalCenter: parent.verticalCenter
        height: label.paintedHeight + Theme.windowMargin
        width: parent.width - closeButton.width
        radius: 4
        z: 2
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

        Rectangle {
            anchors.fill: parent
            anchors.margins: -3
            border.color: alertColor.focusStroke // TODO: Figure these out
            border.width: parent.activeFocus ? 4 : 0
            color: "transparent"
            opacity: parent.activeFocus ? 1 : 0
            radius: 7
        }

        Label {
            id: label

            anchors.centerIn: parent
            text: alertBox.alertText + " " + "<b><u>" + alertLinkText + "</b></u>"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.white
            width: alertAction.width - Theme.windowMargin
            wrapMode: Label.WordWrap
        }

    }

    Rectangle {
        anchors.fill: closeButton
        anchors.margins: -3
        border.color: alertColor.focusStroke // TODO: Figure these out
        border.width: closeButton.activeFocus ? 4 : 0
        color: "transparent"
        opacity: closeButton.activeFocus ? 1 : 0
        radius: 7
        z: 2
    }

    VPNButtonBase {
        id: closeButton

        targetEl: backgroundRect
        bgColor: alertColor
        height: parent.height
        width: 40
        clip: true
        anchors.right: parent.right
        anchors.rightMargin: 0
        Accessible.name: "Close"
        onClicked: {
            alertBox.visible = false;
            if (alertType === "update") {
                parent.updatePageLayout();
                return VPN.hideUpdateRecommendedAlert();
            }
            return VPN.hideAlert();
        }

        // Hack to create the two right angle corners
        // where closeButton meets alertAction
        Rectangle {
            id: backgroundRect

            height: parent.height
            width: parent.width + 10
            anchors.left: closeButton.left
            anchors.leftMargin: -10
            radius: 4

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

    }

    Behavior on color {
        ColorAnimation {
            duration: 200
        }

    }

}
