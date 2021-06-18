/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0
import "../themes/themes.js" as Theme

Item {
    id: alertBox

    property var alertType: ""
    property var alertColor: Theme.redButton
    property var textColor: "#ffffff"
    property var alertText: ""
    property var alertLinkText: ""
    property bool isLayout: false
    property var alertHeight: Math.max(Theme.rowHeight, (label.paintedHeight + (Theme.windowMargin * 1.25)))

    VPNDropShadow {
        anchors.fill: parent
        source: parent
        opacity: .1
        state: "overwrite-state"
        z: -1
    }

    z: 3
    Layout.minimumHeight: alertHeight
    Layout.maximumHeight: alertHeight
    Layout.fillWidth: isLayout

    onVisibleChanged: {
        if (visible) {
            showAlert.start();
        }
    }

    Component.onCompleted:  {
        if (!isLayout) {
            height = alertHeight;
            width = Math.min(window.width - Theme.windowMargin, Theme.maxHorizontalContentWidth);
            y = fullscreenRequired()? iosSafeAreaTopMargin.height + Theme.windowMargin : Theme.windowMargin;
            anchors.horizontalCenter = parent.horizontalCenter;
            anchors.margins = Theme.windowMargin / 2;
        }
    }

    VPNButtonBase {
        id: alertAction

        anchors.fill: alertBox
        radius: Theme.cornerRadius
        onClicked: {
            switch (alertType) {
            case ("update"):
                stackview.push("../views/ViewUpdate.qml", StackView.Immediate);
                break;
            case ("authentication-failed"):
                VPN.authenticate();
                break;
            case ("backend-service"):
                VPN.backendServiceRestore();
                break;
            case ("survey"):
                VPNSurveyModel.openCurrentSurvey();
                break;
            case ("connection-failed"):
                // fall-through
            case ("no-connection"):
                // fall-through
            case ("subscription-failed"):
                // fall-through
            case ("geoip-restriction"):
                // fall-through
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
            height: label.paintedHeight + Theme.windowMargin
            anchors.left: alertAction.left
            width: alertAction.width - Theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            Label {
                id: label
                 anchors.centerIn: parent
                 text: alertBox.alertText + " " + "<b><u>" + alertLinkText + "</b></u>"
                 horizontalAlignment: Text.AlignHCenter
                 font.pixelSize: Theme.fontSizeSmall
                 color: textColor
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
        radius: Theme.cornerRadius
    }

    VPNButtonBase {
        // Hack to create the two right angle corners
        // where closeButton meets alertAction

        id: closeButton

        height: alertBox.height
        width: Theme.rowHeight
        clip: true
        anchors.right: alertBox.right
        anchors.rightMargin: 0
        radius: Theme.cornerRadius
        Accessible.name: "Close"
        onClicked: {
            if (alertType === "update") {
                closeAlert.start();
                return VPN.hideUpdateRecommendedAlert();
            }
            if (alertType === "survey") {
                closeAlert.start();
                return VPNSurveyModel.dismissCurrentSurvey();
            }
            closeAlert.start();
            return VPN.hideAlert();
        }

        VPNFocusBorder {
            anchors.fill: closeButton
            border.color: alertColor.focusBorder
            opacity: closeButton.activeFocus ? 1 : 0
            radius: Theme.cornerRadius
            z: 1
        }

        Rectangle {
            id: backgroundRect

            height: parent.height
            width: parent.width + 10
            anchors.left: closeButton.left
            anchors.leftMargin: -10
            radius: Theme.cornerRadius
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

            source: textColor === "#ffffff" ? "../resources/close-white.svg" : "../resources/close-dark.svg"
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
        radius: Theme.cornerRadius
    }

    SequentialAnimation {
        id: showAlert

        PropertyAnimation {
            target: alertBox
            property: isLayout ? "Layout.minimumHeight" : "height"
            to: alertHeight
            duration: 60
        }

        PropertyAnimation {
            target: alertBox
            property: "opacity"
            to: 1
            duration: 100
        }
    }

    SequentialAnimation {
        property var closeTarget
        id: closeAlert

        PropertyAnimation {
            target: alertBox
            property: "opacity"
            to: 0
            duration: 60
        }
        PropertyAnimation {
            target: alertBox
            property: isLayout ? "Layout.minimumHeight" : "height"
            to: 0
            duration: 60
        }
        PropertyAction {
            target: alertBox
            property: "visible"
            value: "false"
        }
    }

}
