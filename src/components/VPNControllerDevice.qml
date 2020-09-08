import QtQuick 2.0
import QtQuick.Layouts 1.0

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

RowLayout {
    id: deviceView
    spacing: 0
    width: parent.width

    state: VPNController.state !== VPNController.StateDeviceLimit ? "active": "deviceLimit"

    states: [
        State {
            name: "active" // normal mode
            PropertyChanges {
                target: label
                text: qsTr("%1 of %2").arg(VPNDeviceModel.activeDevices).arg(
                          VPNUser.maxDevices)
            }
        },
        State {
            name: "deviceLimit" // device limit mode
            PropertyChanges {
                target: label
                text: qsTr("%1 of %2").arg(VPNDeviceModel.activeDevices + 1).arg(
                          VPNUser.maxDevices)
            }
            PropertyChanges {
                target: icon
                // TODO: this should be an alert icon
                source: "../resources/chevron.svg"
            }
        }
    ]

    VPNIcon {
        source: "../resources/devices.svg"
        Layout.rightMargin: Theme.iconSize
        Layout.leftMargin: Theme.hSpacing
    }

    VPNBoldLabel {
        text: qsTr("My devices")
    }

    Item {
        Layout.fillWidth: true
    }

    VPNLightLabel {
        id: label
        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewDevices.qml")
        }
    }

    VPNChevron {
        id: icon
        MouseArea {
            anchors.fill: parent
            onClicked: stackview.push("../views/ViewDevices.qml")
        }
    }
}
