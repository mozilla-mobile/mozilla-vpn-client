/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import "../themes/themes.js" as Theme


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

    Layout.minimumHeight: style.alertHeight
    Layout.maximumHeight: style.alertHeight
    Layout.fillWidth: isLayout
    color: style.alertColor
    visible: false
    radius: Theme.cornerRadius
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
        readonly property string darkCloseIcon: "../resources/close-dark.svg"
        readonly property string whiteCloseIcon: "../resources/close-white.svg"
        property var alertColor: "black";
        property var alertHoverColor: "gray";
        property var alertClickColor: "white";
        property var fontColor: "yellow"
        property var fontSize: 13
        property var lineHeight: 21
        property var borderRadius: 4
        property var alertHeight: Math.max(Theme.rowHeight, (label.paintedHeight + (Theme.windowMargin * 1.25)))
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
                colorScheme: Theme.blueButton
            }

            PropertyChanges {
                target: style;
                alertColor: Theme.blue
                alertHoverColor: Theme.blueHovered
                alertClickColor: Theme.bluePressed
                fontColor: "#FFFFFF"
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.success
            PropertyChanges {
                target: focusIndicators
                colorScheme: Theme.greenAlert
            }

            PropertyChanges {
                target: style;
                alertColor: Theme.greenAlert.defaultColor
                alertHoverColor: Theme.greenAlert.buttonHovered
                alertClickColor: Theme.greenAlert.buttonPressed
                fontColor: Theme.fontColorDark
                closeIcon: darkCloseIcon
            }
        },

        State{
            name: stateNames.error

            PropertyChanges {
                target: focusIndicators
                colorScheme: Theme.redButton
            }

            PropertyChanges {
                target: style;
                alertColor: Theme.red
                alertHoverColor: Theme.redHovered
                alertClickColor: Theme.redPressed
                fontColor: "#FFFFFF"
                closeIcon: whiteCloseIcon
            }
        },
        State{
            name: stateNames.warning
            PropertyChanges {
                target: focusIndicators
                colorScheme: Theme.warningAlertfocusIndicators
            }

            PropertyChanges {
                target: style;
                alertColor: Theme.orange
                alertHoverColor: Theme.orangeHovered
                alertClickColor: Theme.orangePressed
                fontColor: Theme.fontColorDark
                closeIcon: darkCloseIcon
            }
        }
    ]

    Timer {
          interval: alertBox.duration
          id: autoHideTimer
          running: false
          repeat: false
          onTriggered: { closeAlert.start();}
    }

    DropShadow {
        anchors.fill: parent
        source: parent
        opacity: .1
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
        radius: Theme.cornerRadius
        enabled: alertActionText != ""
        onClicked: {
            if(alertActionText != ""){
              // Only Trigger an Action,
              // if we have an actionable text
              alertBox.onActionPressed();
              closeAlert.start();
            }
        }

        Rectangle {
            id: labelWrapper
            height: label.paintedHeight + Theme.windowMargin
            color: "transparent"
            anchors.left: alertAction.left
            width: alertAction.width - Theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            Label {
                property var whiteSpaceCharacter: Qt.platform.os === "windows" || Qt.platform.os === "android" ? " " : "\u0001"
                id: label
                anchors.centerIn: parent
                text: alertBox.alertText + whiteSpaceCharacter + "<b><u>"  + alertBox.alertActionText + "</b></u>"
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: Theme.fontSizeSmall
                color: style.fontColor
                width: labelWrapper.width - Theme.windowMargin
                wrapMode: Label.WordWrap
            }
        }

        VPNMouseArea {
            anchors.leftMargin: closeButton.width
        }

        state: Theme.uiState.stateDefault
        states: [
            State {
                name: Theme.uiState.stateDefault
                PropertyChanges {target: alertBox; color: style.alertColor }
            },
            State {
                name: Theme.uiState.statePressed
                PropertyChanges {target: alertBox; color: style.alertClickColor }
            },
            State {
                name: Theme.uiState.stateHovered
                PropertyChanges {target: alertBox; color: style.alertHoverColor }
            }
        ]
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
            radius: Theme.cornerRadius
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
                    name: Theme.uiState.stateDefault
                    PropertyChanges {target: backgroundRect; color: style.alertColor }
                },
                State {
                    name: Theme.uiState.statePressed
                    PropertyChanges {target: backgroundRect; color: style.alertClickColor }
                },
                State {
                    name: Theme.uiState.stateHovered
                    PropertyChanges {target: backgroundRect; color: style.alertHoverColor }
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
        border.color: colorScheme ? colorScheme.focusOutline : "transparent"
        border.width: 3
        visible: closeButton.activeFocus || alertAction.activeFocus
        color: "transparent"
        radius: Theme.cornerRadius + (anchors.margins * -1)

        Rectangle {
            color: "transparent"
            border.width: 2
            border.color: parent.colorScheme ? parent.colorScheme.focusBorder : "transparent"
            radius: Theme.cornerRadius
            anchors.fill: parent
            anchors.margins: 3
        }
    }

    SequentialAnimation {
        id: showAlert
        ScriptAction { script: show();}

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
            width = Math.min(window.width - Theme.windowMargin, Theme.maxHorizontalContentWidth);
            y = fullscreenRequired()? iosSafeAreaTopMargin.height + Theme.windowMargin : Theme.windowMargin;
            anchors.horizontalCenter = parent.horizontalCenter;
            anchors.margins = Theme.windowMargin / 2;
        }
        if(alertBox.duration > 0){
            console.log("Toasbox timer start")
            autoHideTimer.start()
        }
    }

    function remove(){

        if(alertBox.destructive){
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
