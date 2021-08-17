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


    color: style.alertColor
    Behavior on color {
        ColorAnimation {
            duration: 200
        }
    }
    radius: Theme.cornerRadius
    height : style.alertHeight;
    z: 3
    Layout.minimumHeight: style.alertHeight
    Layout.maximumHeight: style.alertHeight
    Layout.fillWidth: isLayout

    onVisibleChanged: {
        if (visible) {
            showAlert.start();
        }
    }
    Component.onCompleted:  {
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
                target: style;
                alertColor: Theme.orange
                alertHoverColor: Theme.orangeHovered
                alertClickColor: Theme.organgePressed
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
                id: label
                 anchors.centerIn: parent
                 text: alertBox.alertText + " " + "<b><u>" + alertBox.alertActionText + "</b></u>"
                 horizontalAlignment: Text.AlignHCenter
                 font.pixelSize: Theme.fontSizeSmall
                 color: style.fontColor
                 width: labelWrapper.width - Theme.windowMargin
                 wrapMode: Label.WordWrap
             }

        }

        VPNMouseArea {}
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

    VPNFocusOutline {
        focusColorScheme: style.alertHoverColor
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
            closeAlert.start();
            alertBox.onClosePressed();
        }

        VPNFocusBorder {
            anchors.fill: closeButton
            border.color: style.focusBorder
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
            color: style.alertColor
            clip: true
            state: closeButton.state

            Behavior on color {
                ColorAnimation {
                    duration: 200
                }
            }

        }

        Image {
            id: alertBoxClose
            source: style.closeIcon
            sourceSize.width: 12
            sourceSize.height: 12
            anchors.centerIn: closeButton
        }

        VPNMouseArea {}
        state: Theme.uiState.stateDefault
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

    VPNFocusBorder {
        anchors.fill: alertBox
        border.color: style.focusBorder
        opacity: alertAction.activeFocus ? 1 : 0
        radius: Theme.cornerRadius
    }

    SequentialAnimation {
        id: showAlert

        PropertyAnimation {
            target: alertBox
            property: isLayout ? "Layout.minimumHeight" : "height"
            to: style.alertHeight
            duration: 60
        }

        PropertyAnimation {
            target: alertBox
            property: "opacity"
            to: 1
            duration: 100
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
        ScriptAction { script: remove();}
    }

}
