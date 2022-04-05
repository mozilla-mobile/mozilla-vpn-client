/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import compat 0.1

Rectangle {
    id: alertBox

    // Defines what type of alert this is
    property var alertType: stateNames.info
    // Contains all accepted values for alertType
    readonly property var alertTypes: stateNames
    // Determines if the toast should fit into the layout or put itself on the top
    property var isLayout: false
    // Callback when the action was pressed,
    // (does nothing if  no action text is defined)
    property var onActionPressed:function(){}
    // Callback when the close button was pressed
    property var onClosePressed:function(){}
    // Text of the toast
    property var alertText: "Some Alert"
    // Text of the Action (in bold)
    property var alertActionText: ""
    // Auto Hide the Alert after X secounds (0 == infinte)
    property var duration: 0
    // Delete itself after closing
    property var destructive: false
    // Fixes the Y on show() if the alert does not use Layout
    property var setY: 0

    Layout.minimumHeight: style.alertHeight
    Layout.maximumHeight: style.alertHeight
    Layout.fillWidth: isLayout
    color: style.alertColor
    visible: false
    radius: VPNTheme.theme.cornerRadius
    height : style.alertHeight;
    z: 3

    onVisibleChanged: {
        if (visible) {
            showAlert.start();
        }
    }
    // Private Properties, will be changed depnding on alertType
    QtObject {
        id: style
        readonly property string darkCloseIcon: "qrc:/nebula/resources/close-dark.svg"
        readonly property string whiteCloseIcon: "qrc:/nebula/resources/close-white.svg"
        property var alertColor: "black";
        property var alertHoverColor: "gray";
        property var alertClickColor: "white";
        property var fontColor: "yellow"
        property var fontSize: 13
        property var lineHeight: 21
        property var borderRadius: 4
        property var alertHeight: Math.max(VPNTheme.theme.rowHeight, (label.paintedHeight + (VPNTheme.theme.windowMargin * 1.25)))
        property var closeIcon : darkCloseIcon
        property var focusBorder: alertColor
    }

    // Possible Alert Types
    QtObject {
        id: stateNames
        readonly property string info: "info"
        readonly property string success: "success"
        readonly property string error: "error"
        readonly property string warning: "warning"
    }

    state: alertType
    states:[
        State{
            name: stateNames.info
            PropertyChanges {
                target: focusIndicators
                colorScheme: VPNTheme.theme.blueButton
            }

            PropertyChanges {
                target: style;
                alertColor: VPNTheme.theme.blue
                alertHoverColor: VPNTheme.theme.blueHovered
                alertClickColor: VPNTheme.theme.bluePressed
                fontColor: "#FFFFFF"
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.success
            PropertyChanges {
                target: focusIndicators
                colorScheme: VPNTheme.theme.greenAlert
            }

            PropertyChanges {
                target: style;
                alertColor: VPNTheme.theme.greenAlert.defaultColor
                alertHoverColor: VPNTheme.theme.greenAlert.buttonHovered
                alertClickColor: VPNTheme.theme.greenAlert.buttonPressed
                fontColor: VPNTheme.theme.fontColorDark
                closeIcon: darkCloseIcon
            }
        },
        State{
            name: stateNames.error

            PropertyChanges {
                target: focusIndicators
                colorScheme: VPNTheme.theme.redButton
            }

            PropertyChanges {
                target: style;
                alertColor: VPNTheme.theme.red
                alertHoverColor: VPNTheme.theme.redHovered
                alertClickColor: VPNTheme.theme.redPressed
                fontColor: "#FFFFFF"
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.warning
            PropertyChanges {
                target: focusIndicators
                colorScheme: VPNTheme.theme.warningAlertfocusIndicators
            }

            PropertyChanges {
                target: style;
                alertColor: VPNTheme.theme.orange
                alertHoverColor: VPNTheme.theme.orangeHovered
                alertClickColor: VPNTheme.theme.orangePressed
                fontColor: VPNTheme.theme.fontColorDark
                closeIcon: darkCloseIcon
            }
        }
    ]

    Timer {
        interval: alertBox.duration
        id: autoHideTimer
        running: false
        repeat: false
        onTriggered: {
            closeAlert.start();
        }
    }

    VPNDropShadow {
        anchors.fill: parent
        source: parent
        opacity: 0.1
        state: "overwrite-state"
        z: -1
        id: dropShadow
        horizontalOffset: 1
        verticalOffset: 1
        radius: 5.5
        color: "#0C0C0D"
   }

    VPNButtonBase {
        id: alertAction
        anchors.fill: alertBox
        radius: VPNTheme.theme.cornerRadius
        enabled: alertActionText !== ""
        onClicked: {
            if (alertActionText !== ""){
              // Only Trigger an Action,
              // if we have an actionable text
              alertBox.onActionPressed();
              closeAlert.start();
            }
        }

        Rectangle {
            id: labelWrapper
            height: label.paintedHeight + VPNTheme.theme.windowMargin
            color: VPNTheme.theme.transparent
            anchors.left: alertAction.left
            width: alertAction.width - VPNTheme.theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            Label {
                id: label
                anchors.centerIn: parent
                text: alertBox.alertText + " " + "<b><u>" + alertBox.alertActionText + "</b></u>"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: VPNTheme.theme.fontSizeSmall
                color: style.fontColor
                width: labelWrapper.width - VPNTheme.theme.windowMargin
                wrapMode: Label.WordWrap
            }
        }

        VPNMouseArea {
            anchors.leftMargin: closeButton.width
        }

        state: VPNTheme.theme.uiState.stateDefault
        states: [
            State {
                name: VPNTheme.theme.uiState.stateDefault
                PropertyChanges {
                    target: alertBox
                    color: style.alertColor
                }
            },
            State {
                name: VPNTheme.theme.uiState.statePressed
                PropertyChanges {
                    target: alertBox
                    color: style.alertClickColor
                }
            },
            State {
                name: VPNTheme.theme.uiState.stateHovered
                PropertyChanges {
                    target: alertBox
                    color: style.alertHoverColor
                }
            }
        ]
    }

    VPNButtonBase {
        // Hack to create the two right angle corners
        // where closeButton meets alertAction

        id: closeButton
        height: alertBox.height
        width: VPNTheme.theme.rowHeight
        clip: true
        anchors.right: alertBox.right
        anchors.rightMargin: 0
        radius: VPNTheme.theme.cornerRadius
        Accessible.name: "Close"
        onClicked: {
            closeAlert.start();
            alertBox.onClosePressed();
        }

        VPNMouseArea { }

        Rectangle {
            id: backgroundRect

            height: parent.height
            width: parent.width + 10
            anchors.left: closeButton.left
            anchors.leftMargin: -10
            radius: VPNTheme.theme.cornerRadius
            color: style.alertColor
            clip: true
            state: parent.state
            opacity: alertBox.opacity === 1 ? 1 : 0

            Behavior on color {
                ColorAnimation {
                    duration: 100
                }
            }
            states: [
                State {
                    name: VPNTheme.theme.uiState.stateDefault
                    PropertyChanges {
                        target: backgroundRect
                        color: style.alertColor
                    }
                },
                State {
                    name: VPNTheme.theme.uiState.statePressed
                    PropertyChanges {
                        target: backgroundRect
                        color: style.alertClickColor
                    }
                },
                State {
                    name: VPNTheme.theme.uiState.stateHovered
                    PropertyChanges {
                        target: backgroundRect
                        color: style.alertHoverColor
                    }
                }
            ]
        }

        Image {
            id: alertBoxClose
            source: style.closeIcon
            sourceSize.width: 12
            sourceSize.height: 12
            anchors.centerIn: closeButton
        }
    }

    Rectangle {
        property var colorScheme
        id: focusIndicators
        anchors.fill: closeButton.activeFocus ? closeButton : alertAction
        anchors.margins: -3
        border.color: colorScheme ? colorScheme.focusOutline : VPNTheme.theme.transparent
        border.width: 3
        visible: closeButton.activeFocus || alertAction.activeFocus
        color: VPNTheme.theme.transparent
        radius: VPNTheme.theme.cornerRadius + (anchors.margins * -1)

        Rectangle {
            color: VPNTheme.theme.transparent
            border.width: 2
            border.color: parent.colorScheme ? parent.colorScheme.focusBorder : VPNTheme.theme.transparent
            radius: VPNTheme.theme.cornerRadius
            anchors.fill: parent
            anchors.margins: 3
        }
    }

    SequentialAnimation {
        id: showAlert
        ScriptAction { script: show(); }

        PropertyAnimation {
            targets: alertBox
            property: "opacity"
            from: 0
            to: 1
            duration: 100
        }
    }
    function show() {
        if (!isLayout) {
            height = style.alertHeight;
            width = window.width - VPNTheme.theme.windowMargin * 2;
            if (setY > 0) {
                y = setY;
            } else {
                y = fullscreenRequired() ? iosSafeAreaTopMargin.height + VPNTheme.theme.windowMargin : VPNTheme.theme.windowMargin;
            }
            anchors.horizontalCenter = parent.horizontalCenter;
            anchors.margins = VPNTheme.theme.windowMargin / 2;
        }
        if(alertBox.duration > 0){
            console.log("Toastbox timer start")
            autoHideTimer.start()
        }
    }

    function remove() {
        if (alertBox.destructive){
            alertBox.destroy(100)
        }
    }

    SequentialAnimation {
        property var closeTarget
        id: closeAlert
        ScriptAction { script: show(); }
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
        ScriptAction { script: remove(); }
    }

}
