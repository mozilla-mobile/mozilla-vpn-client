import QtQuick 2.0

import Mozilla.VPN 1.0

Rectangle {
    id: alertBox
    state: VPN.alert
    visible: VPN.alert !== VPN.NoAlert

    states: [
        State {
            name: VPN.NoAlert
        },

        State {
            name: VPN.AuthenticationFailedAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Authentication error. <u><b>Try again</b></u>")
            }
        },

        State {
            name: VPN.ConnectionFailedAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Unable to connect. <u><b>Try again</b></u>");
            }
        },

        State {
            name: VPN.LogoutAlert
            PropertyChanges {
                target: alertText
                text: qsTr("Signed out and device removed");
            }
        },

        State {
            name: VPN.NoConnectionAlert
            PropertyChanges {
                target: alertText
                text: qsTr("No internet connection. <u><b>Try again</b></u>");
            }
        }
    ]

    color: "#FF4F5E"
    height: 40
    width: mainView.width - 16
    y: mainView.height - 48
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
