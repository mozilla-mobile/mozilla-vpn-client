import QtQuick 2.0

import Mozilla.VPN 1.0

Rectangle {
    id: alertBox
    state: VPN.alert

    states: [
        State {
            name: VPN.NoAlert
            PropertyChanges {
                target: alertBox
                opacity: 0
                visible: false
            }
        },

        State {
            name: VPN.AuthenticationFailedAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Authentication error. <u><b>Try again</b></u>")
            }
            PropertyChanges {
                target: alertBox
                opacity: 1
                visible: true
            }
        },

        State {
            name: VPN.ConnectionFailedAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Unable to connect. <u><b>Try again</b></u>");
            }
            PropertyChanges {
                target: alertBox
                opacity: 1
                visible: true
            }
        },

        State {
            name: VPN.LogoutAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Signed out and device removed");
            }
            PropertyChanges {
                target: alertBox
                opacity: 1
                visible: true
            }
        },

        State {
            name: VPN.NoConnectionAlert
            PropertyChanges {
                target: alertText
                text: qsTr("No internet connection. <u><b>Try again</b></u>");
            }
            PropertyChanges {
                target: alertBox
                opacity: 1
                visible: true
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                duration: 300
                properties: "opacity"
            }
        }
    ]

    color: "#FF4F5E"
    height: 40
    width: parent.width - 16
    y: parent.height - 48
    x: 8
    radius: 4
    anchors.margins: 8

    Text {
        id: alertText
        font.pixelSize: 13
        color: "#FFFFFF"
        anchors.centerIn: alertBox
    }

    Image {
        id: alertBoxClose
        source: "../resources/close-white.svg"
        sourceSize.width: 12
        sourceSize.height: 12
        anchors.right: alertBox.right
        anchors.top: alertBox.top
        anchors.topMargin: 14
        anchors.rightMargin: 14
    }

    MouseArea {
        anchors.fill: alertBoxClose
        onClicked: VPN.hideAlert()
    }
}
