/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import compat 0.1

Rectangle {
    id: alertBox

    // Defines what type of alert this is
    property string alertType: stateNames.info
    // Contains all accepted values for alertType
    readonly property var alertTypes: stateNames
    // Determines if the toast should fit into the layout or put itself on the top
    property bool isLayout: false
    // Callback when the action was pressed,
    // (does nothing if  no action text is defined)
    property var onActionPressed:function(){}
    // Callback when the close button was pressed
    property var onClosePressed:function(){}
    // Text of the toast
    property string alertText: "Some Alert"
    // Text of the Action (in bold)
    property string alertActionText: ""
    // Auto Hide the Alert after X secounds (0 == infinte)
    property real duration: 0
    // Delete itself after closing
    property bool destructive: false
    // Fixes the Y on show() if the alert does not use Layout
    property real setY: 0

    Layout.minimumHeight: style.alertHeight
    Layout.maximumHeight: style.alertHeight
    Layout.fillWidth: isLayout
    color: style.alertColor
    visible: false
    radius: MZTheme.theme.cornerRadius
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
        property var alertColor: MZTheme.colors.black
        property var alertHoverColor: MZTheme.colors.grey30
        property var alertClickColor: MZTheme.colors.white
        property var fontColor: MZTheme.colors.yellow10
        property var fontSize: 13
        property var lineHeight: 21
        property var borderRadius: 4
        property var alertHeight: Math.max(MZTheme.theme.rowHeight, (label.paintedHeight + (MZTheme.theme.windowMargin * 1.25)))
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
                colorScheme: MZTheme.theme.blueButton
            }

            PropertyChanges {
                target: style;
                alertColor: MZTheme.theme.blue
                alertHoverColor: MZTheme.theme.blueHovered
                alertClickColor: MZTheme.theme.bluePressed
                fontcolor: MZTheme.colors.white
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.success
            PropertyChanges {
                target: focusIndicators
                colorScheme: MZTheme.theme.greenAlert
            }

            PropertyChanges {
                target: style;
                alertColor: MZTheme.theme.greenAlert.defaultColor
                alertHoverColor: MZTheme.theme.greenAlert.buttonHovered
                alertClickColor: MZTheme.theme.greenAlert.buttonPressed
                fontColor: MZTheme.theme.fontColorDark
                closeIcon: darkCloseIcon
            }
        },
        State{
            name: stateNames.error

            PropertyChanges {
                target: focusIndicators
                colorScheme: MZTheme.theme.redButton
            }

            PropertyChanges {
                target: style;
                alertColor: MZTheme.theme.red
                alertHoverColor: MZTheme.theme.redHovered
                alertClickColor: MZTheme.theme.redPressed
                fontcolor: MZTheme.colors.white
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.warning
            PropertyChanges {
                target: focusIndicators
                colorScheme: MZTheme.theme.warningAlertfocusIndicators
            }

            PropertyChanges {
                target: style
                alertColor: MZTheme.theme.orange
                alertHoverColor: MZTheme.theme.orangeHovered
                alertClickColor: MZTheme.theme.orangePressed
                fontColor: MZTheme.theme.fontColorDark
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

    MZDropShadow {
        anchors.fill: parent
        source: parent
        opacity: 0.1
        state: "overwrite-state"
        z: -1
        id: dropShadow
        horizontalOffset: 1
        verticalOffset: 1
        radius: 5.5
        color: MZTheme.colors.grey60
   }

    MZButtonBase {
        id: alertAction
        anchors.fill: alertBox
        radius: MZTheme.theme.cornerRadius
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
            height: label.paintedHeight + MZTheme.theme.windowMargin
            color: MZTheme.theme.transparent
            anchors.left: alertAction.left
            width: alertAction.width - MZTheme.theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            Label {
                id: label
                Accessible.role: Accessible.StaticText
                Accessible.ignored: !visible
                anchors.centerIn: parent
                text: alertBox.alertText + " " + "<b><u>" + alertBox.alertActionText + "</b></u>"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: MZTheme.theme.fontSizeSmall
                color: style.fontColor
                width: labelWrapper.width - MZTheme.theme.windowMargin
                wrapMode: Label.WordWrap
            }
        }

        MZMouseArea {
            anchors.leftMargin: closeButton.width
        }

        state: MZTheme.theme.uiState.stateDefault
        states: [
            State {
                name: MZTheme.theme.uiState.stateDefault
                PropertyChanges {
                    target: alertBox
                    color: style.alertColor
                }
            },
            State {
                name: MZTheme.theme.uiState.statePressed
                PropertyChanges {
                    target: alertBox
                    color: style.alertClickColor
                }
            },
            State {
                name: MZTheme.theme.uiState.stateHovered
                PropertyChanges {
                    target: alertBox
                    color: style.alertHoverColor
                }
            }
        ]
    }

    MZButtonBase {
        // Hack to create the two right angle corners
        // where closeButton meets alertAction

        id: closeButton
        height: alertBox.height
        width: MZTheme.theme.rowHeight
        clip: true
        anchors.right: alertBox.right
        anchors.rightMargin: 0
        radius: MZTheme.theme.cornerRadius
        Accessible.name: "Close"
        onClicked: {
            closeAlert.start();
            alertBox.onClosePressed();
        }

        MZMouseArea { }

        Rectangle {
            id: backgroundRect

            height: parent.height
            width: parent.width + 10
            anchors.left: closeButton.left
            anchors.leftMargin: -10
            radius: MZTheme.theme.cornerRadius
            color: style.alertColor
            clip: true
            state: parent.state
            opacity: alertBox.opacity === 1 ? 1 : 0

            transitions: Transition {
                    ColorAnimation { property: "color"; duration: 100 }
            }
            states: [
                State {
                    name: MZTheme.theme.uiState.stateDefault
                    PropertyChanges {
                        target: backgroundRect
                        color: style.alertColor
                    }
                },
                State {
                    name: MZTheme.theme.uiState.statePressed
                    PropertyChanges {
                        target: backgroundRect
                        color: style.alertClickColor
                    }
                },
                State {
                    name: MZTheme.theme.uiState.stateHovered
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
        border.color: colorScheme ? colorScheme.focusOutline : MZTheme.theme.transparent
        border.width: 3
        visible: closeButton.activeFocus || alertAction.activeFocus
        color: MZTheme.theme.transparent
        radius: MZTheme.theme.cornerRadius + (anchors.margins * -1)

        Rectangle {
            color: MZTheme.theme.transparent
            border.width: 2
            border.color: parent.colorScheme ? parent.colorScheme.focusBorder : MZTheme.theme.transparent
            radius: MZTheme.theme.cornerRadius
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
        ScriptAction { script: onShowCompleted(); }
    }

    function show() {
        if (!isLayout) {
            height = style.alertHeight;
            width = window.width - MZTheme.theme.windowMargin * 2;
            if (setY > 0) {
                y = setY;
            } else {
                y = fullscreenRequired() ? iosSafeAreaTopMargin.height + MZTheme.theme.windowMargin : MZTheme.theme.windowMargin;
            }
            anchors.horizontalCenter = parent.horizontalCenter;
            anchors.margins = MZTheme.theme.windowMargin / 2;
        }
        if(alertBox.duration > 0){
            console.log("Toastbox timer start")
            autoHideTimer.start()
        }
    }

    function onShowCompleted() {
        if (!label.Accessible.ignored)
            MZAccessibleNotification.notify(label, alertBox.alertText + ". " + alertBox.alertActionText);
    }

    function remove() {
        MZErrorHandler.hideAlert();
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
