import QtQuick 2.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

import "../themes/themes.js" as Theme

VPNClickableRow {
    // TODO:
    // 1. the server should be always changable except in DeviceLimit mode.
    // 2. in DeviceLimit mode we should gray out the label
    rowShouldBeDisabled: VPNController.state === VPNController.StateDeviceLimit

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 0

        VPNIconAndLabel {
            icon: "../resources/connection.svg"
            title: qsTr("Select location")
        }

        Item {
            Layout.fillWidth: true
        }

        Image {
            Layout.preferredWidth: 16
            Layout.preferredHeight: 16
            Layout.rightMargin: 8
            fillMode: Image.PreserveAspectFit
            source: "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase(
                        ) + ".png"
        }

        VPNLightLabel {
            text: VPNCurrentServer.city
            Layout.rightMargin: 8
        }

        VPNChevron { }
    }
}
