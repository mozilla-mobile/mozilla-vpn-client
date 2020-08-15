import QtQuick 2.12
import QtQuick.Window 2.12
import Qt.labs.platform 1.1

import Mozilla.VPN 1.0

Window {
    id: window
    visible: true
    width: 360
    height: 454
    title: qsTr("Mozilla VPN")

    FontLoader { id: vpnFont; source: "qrc:/resources/Metropolis-Regular.otf" }

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
                onTriggered: Qt.quit()
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
                name: "INITIALIZE"
                PropertyChanges {
                    target: loader
                    source: "StateInitialize.qml"
                }
            },

            State {
                name: "CONNECTING"
                PropertyChanges {
                    target: loader
                    source: "StateConnecting.qml"
                }
            }
        ]
    }
}
