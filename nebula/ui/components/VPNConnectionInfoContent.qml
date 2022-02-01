import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Flickable {
    id: root

    height: parent.height
    contentHeight: Math.max(content.height, height)
    width: parent.width

    ColumnLayout {
        id: content

        spacing: 0
        width: parent.width


        // IP Adresses
        VPNInterLabel {
            color: VPNTheme.colors.grey20
            text: "IP: 103.231.88.10"

            anchors.horizontalCenter: content.horizontalCenter
        }
        VPNInterLabel {
            color: VPNTheme.colors.grey20
            text: "IPv6: 2001:ac8:40:b9::a09e"

            anchors.horizontalCenter: content.horizontalCenter
        }

        // Lottie animation
        Rectangle {
            color: "black"
            radius: 50

            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 100
            Layout.preferredWidth: 100
            Layout.minimumHeight: 50
            Layout.minimumWidth: 50
        }

        Rectangle {
            color: VPNTheme.colors.grey20
            height: 1
            Layout.fillWidth: true
        }

        // Bullet list
        VPNTextBlock {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: VPNTheme.colors.white
            text: "At your current speed, here's what your device is optimized for:"
        }
        VPNTextBlock {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: VPNTheme.colors.grey20
            text: "Streaming in 4K"
        }
        VPNTextBlock {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: VPNTheme.colors.grey20
            text: "High-speed downloads"
        }
        VPNTextBlock {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: VPNTheme.colors.grey20
            text: "Online gaming"
        }

        Rectangle {
            color: VPNTheme.colors.grey20
            height: 1
            Layout.fillWidth: true
        }

        // Detailed info section
        VPNConnectionInfoItem {
            title: VPNServerCountryModel.getLocalizedCountryName(VPNCurrentServer.exitCountryCode)
            subtitle: VPNCurrentServer.localizedCityName
            iconPath: "qrc:/nebula/resources/flags/" + VPNCurrentServer.exitCountryCode.toUpperCase() + ".png"
        }
        Rectangle {
            color: VPNTheme.colors.grey20
            height: 1
            Layout.fillWidth: true
        }
        VPNConnectionInfoItem {
            title: "Ping"
            subtitle: "15 ms"
            iconPath: "qrc:/nebula/resources/connection.svg"
        }
        Rectangle {
            color: VPNTheme.colors.grey20
            height: 1
            Layout.fillWidth: true
        }
        VPNConnectionInfoItem {
            title: "Download"
            subtitle: "300.06 Mbps"
            iconPath: "qrc:/nebula/resources/download.svg"
        }
        Rectangle {
            color: VPNTheme.colors.grey20
            height: 1
            Layout.fillWidth: true
        }
        VPNConnectionInfoItem {
            title: "Upload"
            subtitle: "21.60 Mbps"
            iconPath: "qrc:/nebula/resources/upload.svg"
        }

        Rectangle {
            color: "gray"

            anchors.fill: parent
            border.color: "red"
            border.width: 2
            z: -1
        }

    }

}