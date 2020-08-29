import QtQuick 2.0
import Qt.labs.platform 1.1

import Mozilla.VPN 1.0

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
