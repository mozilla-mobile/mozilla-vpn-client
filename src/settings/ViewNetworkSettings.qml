import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme

Flickable {
    VPNMenu {
        id: menu
        title: qsTr("Network settings")
        isSettingsView: true
    }

    ColumnLayout {
        id: checkBoxColumn
        anchors.top: menu.bottom
        Layout.preferredWidth: parent.width
        spacing: 0

        VPNCheckBoxRow {
            id: checkbox1
            labelText: qsTr("IPv6 enabled")
            subLabelText: qsTr("Push the internet forward with the latest version of the Internet Protocol")
            isChecked: (VPNSettings.ipv6)
            onClicked: {
                VPNSettings.ipv6 = !VPNSettings.ipv6
                isChecked = VPNSettings.ipv6
            }
        }

        VPNCheckBoxRow {
            property bool isVPNOff: (VPNController.state === VPNController.StateOff)
            id: localNetwork
            labelText: qsTr("Local network enabled")
            subLabelText: qsTr("Access printers, streaming sticks and all other devices on your local network")
            isChecked: (VPNSettings.localNetwork)
            isEnabled: isVPNOff
            showDivider: isVPNOff

            onClicked: {
                VPNSettings.localNetwork = !VPNSettings.localNetwork
                isChecked = VPNSettings.localNetwork
            }
        }

        RowLayout {
            id: turnVPNOffAlert
            visible: !localNetwork.isVPNOff
            Layout.topMargin: 12
            Layout.leftMargin: 56
            Layout.rightMargin: Theme.windowMargin
            spacing: 0

            VPNIcon {
                id: warningIcon
                source: "../resources/warning.svg"
                sourceSize.height: 14
                sourceSize.width: 14
                Layout.rightMargin: 8
                Layout.leftMargin: 4
            }
            Text {
                text: VPNSettings.localNetwork ? qsTr("VPN must be off before disabling") : qsTr("VPN must be off before enabling")
                color: Theme.red
                font.pixelSize: 13
                font.family: vpnFontInter.name
                wrapMode: Text.Wrap
                verticalAlignment: Qt.AlignVCenter
                width: checkBoxColumn.width - turnVPNOffAlert.Layout.leftMargin - warningIcon.sourceSize.width - warningIcon.Layout.rightMargin - Theme.windowMargin
            }
        }

    }

    ScrollBar.vertical: ScrollBar {}
}
