import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1

import Mozilla.VPN 1.0

Window {
    id: window
    visible: true
    width: 360
    height: 454

    maximumHeight: height
    maximumWidth: width

    minimumHeight: height
    minimumWidth: width

    title: qsTr("Mozilla VPN")
    color: "#F9F9FA"

    FontLoader { id: vpnFont; source: "qrc:/resources/Metropolis-Regular.otf" }
    FontLoader { id: vpnFontSemiBold; source: "qrc:/resources/Metropolis-SemiBold.otf" }

    SystemTrayIcon {
        visible: true
        icon.source: "qrc:/resources/logo.png"

        onActivated: {
            window.show()
            window.raise()
            window.requestActivate()
        }

        menu: Menu {
            MenuItem {
                text: qsTr("Quit")
                onTriggered: VPNController.quit()
            }
        }
    }

    onClosing: {
        window.hide()
        close.accepted = false
    }

    Item {
        id: mainView
        anchors.fill: parent
        state: VPN.state

        Loader {
            id: loader
            anchors.fill:parent
        }

        states: [
            State {
                name: VPN.StateInitialize
                PropertyChanges {
                    target: loader
                    source: "states/StateInitialize.qml"
                }
            },

            State {
                name: VPN.StateAuthenticating
                PropertyChanges {
                    target: loader
                    source: "states/StateAuthenticating.qml"
                }
            },

            State {
                name: VPN.StateMain
                PropertyChanges {
                    target: loader
                    source: "states/StateMain.qml"
                }
            }
        ]

        Rectangle {
            id: alertBox
            visible: VPN.alert !== VPN.NoAlert
            color: "#FF4F5E"
            height: 40
            width: mainView.width - 16
            y: mainView.height - 48
            x: 8
            radius: 4
            anchors.margins: 8

            Text {
                function alertToText() {
                    switch (VPN.alert) {
                    case VPN.NoAlert:
                        return "";

                    case VPN.AuthenticationFailedAlert:
                        return qsTr("Authentication error. <u><b>Try again</b></u>");

                    case VPN.ConnectionFailedAlert:
                        return qsTr("Unable to connect. <u><b>Try again</b></u>");

                    case VPN.LogoutAlert:
                        return qsTr("Signed out and device removed");

                    case VPN.NoConnectionAlert:
                        return qsTr("No internet connection. <u><b>Try again</b></u>");

                    }
                }

                font.pixelSize: 13
                color: "#FFFFFF"
                anchors.centerIn: alertBox
                text: alertToText()
            }

            Image {
                id: alertBoxClose
                source: "resources/close-white.svg"
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
    }
}
