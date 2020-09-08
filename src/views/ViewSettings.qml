import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.0

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
            source: VPNUser.avatar
            sourceSize.width: 80
            sourceSize.height: 80
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 32
            height: 80
        }

        Label {
            text: VPNUser.displayName
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            text: VPNUser.email
            Layout.alignment: Qt.AlignHCenter
        }

        VPNButton {
            id: manageAccountButton
            text: qsTr("Manage account")
            Layout.alignment: Qt.AlignHCenter
            onClicked: VPN.openLink(VPN.LinkAccount)
        }

        // TODO: this should be scrollable
        // TODO: notification
        // TODO: network settings
        // TODO: language
        // TODO: about us

        VPNSettingsItem {
            text: qsTr("About us")
            imageRight: "../resources/chevron.svg"
            imageLeft: "../resources/getHelp.svg"
            onClicked: stackview.push("ViewAboutUs.qml")
        }

        VPNSettingsItem {
            text: qsTr("Get help")
            imageRight: "../resources/chevron.svg"
            imageLeft: "../resources/getHelp.svg"
            onClicked: stackview.push("ViewGetHelp.qml")
        }

        VPNSettingsItem {
            text: qsTr("Give feedback")
            imageRight: "../resources/externalLink.svg"
            imageLeft: "../resources/feedback.svg"
            onClicked: VPN.openLink(VPN.LinkFeedback)
        }

        Label {
            text: qsTr("Sign out")
            Layout.alignment: Qt.AlignHCenter
            color: "red"

            MouseArea {
                anchors.fill: parent
                onClicked: VPNController.logout()
            }
        }
    }
}
