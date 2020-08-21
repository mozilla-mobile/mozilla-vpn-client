import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.11
import Mozilla.VPN 1.0

import "../components"

Item {
    id: settings

    VPNMenu {
        id: menu
        title: qsTr("Settings")
    }

    ColumnLayout {
        width: parent.width
        anchors.top: menu.bottom

        Image {
            source: VPN.user.avatar
            sourceSize.width: 80
            sourceSize.height: 80
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 32
            height: 80
        }

        Label {
            text: VPN.user.displayName
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: VPN.user.email
            Layout.alignment: Qt.AlignHCenter
        }

        VPNButton {
            id: manageAccountButton
            text: qsTr("Manage account")
            Layout.alignment: Qt.AlignHCenter
        }

        // TODO: this should be scrollable
        // TODO: notification
        // TODO: network settings
        // TODO: language
        // TODO: about us

        VPNSettingsItem {
            text: qsTr("Get help")
            onClicked: stackview.push("ViewGetHelp.qml")
        }

        // TODO: feedback

        Label {
            text: qsTr("Sign out")
            Layout.alignment: Qt.AlignHCenter
            color: "red"

            MouseArea {
                anchors.fill: parent
                onClicked: VPN.logout()
            }
        }
    }
}
