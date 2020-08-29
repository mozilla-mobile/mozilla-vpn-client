import QtQuick 2.12
import QtQuick.Window 2.12

import Mozilla.VPN 1.0

import "./components"

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

    FontLoader { id: vpnFont; source: "qrc:/resources/fonts/Metropolis-Regular.otf" }
    FontLoader { id: vpnFontSemiBold; source: "qrc:/resources/fonts/Metropolis-SemiBold.otf" }
    FontLoader { id: vpnFontInter; source: "qrc:/resources/fonts/Inter-UI-Regular.otf" }

    VPNSystemTray { id: vpnSystemTray }

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

        VPNAlert {
            id: alertBox
        }
    }
}
