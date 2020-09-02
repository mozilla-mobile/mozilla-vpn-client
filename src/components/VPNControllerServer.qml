import QtQuick 2.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

RowLayout {
    spacing: 0
    width: parent.width
    height: Theme.vSpacing

    // TODO:
    // 1. the server should be always changable except in DeviceLimit mode.
    // 2. in DeviceLimit mode we should gray out the label

    VPNIcon {
        source: "../resources/connection.svg"
        Layout.rightMargin: Theme.iconSize
        Layout.leftMargin: Theme.hSpacing
    }

    VPNBoldLabel {
        text: qsTr("Select location")
    }

    Item {
        Layout.fillWidth: true
    }

    VPNIcon {
        Layout.rightMargin: 8
        source: "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase(
                    ) + ".png"
    }

    VPNLightLabel {
        text: VPNCurrentServer.city
        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewServers.qml")
        }
    }

    VPNChevron {
        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewServers.qml")
        }
    }
}
