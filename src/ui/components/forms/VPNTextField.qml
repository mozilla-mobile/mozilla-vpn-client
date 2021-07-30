/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../../themes/themes.js" as Theme
import "./../../components"

TextField {
    // TODO Add strings for Accessible.description, Accessible.name
    property bool stateError: false
    property bool forceErrorVisibility: false
    property bool loseFocusOnOutsidePress: true
    property var errorMessageText: ""

    id: textField

    Layout.preferredHeight: Theme.rowHeight
    onActiveFocusChanged: if (focus && typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible) vpnFlickable.ensureVisible(textField)
    selectByMouse: true
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    onTextChanged: {
        stateError = false;
        forceErrorVisibility = false;
    }

    background: VPNInputBackground {
        showError: stateError
    }
    z: 10

    Label {
        id: errorMessage
        text: "Enter a valid email"
        font.pixelSize: 12
        color: Theme.white
        font.family: Theme.fontBoldFamily
        anchors.top: parent.bottom
        anchors.topMargin: Theme.windowMargin / 1.5
        anchors.left: parent.left
        anchors.leftMargin: Theme.windowMargin
        z: 2
        state: errorMessageText.length > 0 && stateError ? "visible" : "hidden"

        background: Rectangle {
            id: errorBg
            anchors.fill: parent
            anchors.margins: -8
            color: Theme.red
            radius: Theme.cornerRadius

            Rectangle {
                color: errorBg.color
                height: 8
                width: 8
                rotation:45
                anchors.top: parent.top
                anchors.topMargin: -height/2
                radius: 1
                anchors.left: parent.left
                anchors.leftMargin: Theme.windowMargin / 1.5
            }
        }

        states: [
            State {
                name: "hidden"
                PropertyChanges {
                    target: errorMessage
                    visible: false
                    opacity: 0
                }
            },

            State {
                name: "visible"
                PropertyChanges {
                    target: errorMessage
                    visible: true
                    opacity: 1
                }
            }

        ]

        transitions: [
            Transition {
                to: "hidden"
                SequentialAnimation {
                    PropertyAnimation {
                        target: errorMessage
                        property: "opacity"
                        to: 0
                        duration: 100
                    }
                    PropertyAction {
                        target: errorMessage
                        property: "visible"
                        value: false
                    }
                }
            },
            Transition {
                to: "visible"
                SequentialAnimation {
                    PropertyAction {
                        target: errorMessage
                        property: "visible"
                        value: true
                    }
                    PropertyAnimation {
                        target: errorMessage
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 100
                    }
                }
            }
        ]
    }
}
