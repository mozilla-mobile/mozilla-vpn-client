/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

Rectangle {
    property var alertType: "";
    property var alertColor: Theme.redButton;
    property var alertText: "";
    property var alertLinkText: "";

    id: alertBox
    color: alertColor.defaultColor
    height: 40
    width: parent.width - Theme.windowMargin
    y: parent.height - 48
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.margins: Theme.windowMargin / 2
    radius: 4

    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }

    VPNButtonBase {
        bgColor: alertColor
        enableHover: alertLinkText.length > 0
        targetEl: alertBox

        id: alertAction
        height: parent.height
        width: parent.width - closeButton.width
        radius: 4
        z: 2

        Rectangle {
            anchors.fill: parent
             anchors.margins: -3
             border.color: alertColor.focusStroke // TODO: Figure these out
             border.width: parent.activeFocus ? 4 : 0;
             color: "transparent"
             opacity: parent.activeFocus ? 1 : 0;
             radius: 7
         }

        onClicked: {
            switch(alertType) {

            case ("update"):
                stackview.push("../views/ViewUpdate.qml", StackView.Immediate)
                break;

            case ("authentication-failed"):
                VPN.authenticate();
                break;

            case ("connection-failed"):
                // TODO Try again

            case ("no-connection"):
                // TODO Try again

            case ("background-service"):
                // TODO Restore

            default:
                VPN.hideAlert();
            }
        }

        Label {
            anchors.centerIn: parent
            text: alertBox.alertText + " " + "<b><u>"+ alertLinkText + "</b></u>"
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.white
        }
    }

    Rectangle {
        anchors.fill: closeButton
        anchors.margins: -3
        border.color: alertColor.focusStroke // TODO: Figure these out
        border.width: closeButton.activeFocus ? 4 : 0;
        color: "transparent"
        opacity: closeButton.activeFocus ? 1 : 0;
        radius: 7
        z: 2
     }

    VPNButtonBase {
        targetEl: backgroundRect
        bgColor: alertColor
        id: closeButton

        height: parent.height
        width: parent.height
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
            width: parent.height + 10
            anchors.left: closeButton.left
            anchors.leftMargin: - 10
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
}
