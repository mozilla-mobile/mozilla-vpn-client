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

        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0
        width: parent.width - VPNTheme.theme.windowMargin * 2

        // IP Adresses
        VPNInterLabel {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
            Layout.topMargin: VPNTheme.theme.windowMargin * 1.5

            color: VPNTheme.colors.grey20
            text: "IP: 103.231.88.10"
        }
        VPNInterLabel {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: VPNTheme.theme.listSpacing

            color: VPNTheme.colors.grey20
            text: "IPv6: 2001:ac8:40:b9::a09e"
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

        // Bullet list
        VPNTextBlock {
            Layout.topMargin: VPNTheme.theme.listSpacing
            Layout.fillWidth: true
            color: VPNTheme.colors.white
            text: "At your current speed, here's what your device is optimized for:"
            wrapMode: Text.WordWrap
        }

        Row {
            Layout.fillWidth: true

            VPNCheckmark {
                color: VPNTheme.colors.secondary
                height: 24
                width: 24
            }
            VPNTextBlock {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2
                color: VPNTheme.colors.grey20
                text: "Streaming in 4K"
                wrapMode: Text.WordWrap
            }
        }

        Row {
            Layout.fillWidth: true

            VPNCheckmark {
                color: VPNTheme.colors.secondary
                height: 24
                width: 24
            }
            VPNTextBlock {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2
                color: VPNTheme.colors.grey20
                text: "High-speed downloads"
                wrapMode: Text.WordWrap
            }
        }

        Row {
            Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
            Layout.fillWidth: true

            VPNCheckmark {
                color: VPNTheme.colors.secondary
                height: 24
                width: 24
            }
            VPNTextBlock {
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: 2
                color: VPNTheme.colors.grey20
                text: "Online gaming"
                wrapMode: Text.WordWrap
            }
        }

        // Detailed info section
        VPNConnectionInfoItem {
            title: VPNServerCountryModel.getLocalizedCountryName(VPNCurrentServer.exitCountryCode)
            subtitle: VPNCurrentServer.localizedCityName
            iconPath: "qrc:/nebula/resources/flags/" + VPNCurrentServer.exitCountryCode.toUpperCase() + ".png"
            isFlagIcon: true
        }
        Rectangle {
            Layout.fillWidth: true

            color: VPNTheme.colors.white
            height: 1
            opacity: 0.2
        }
        VPNConnectionInfoItem {
            title: "Ping"
            subtitle: "15 ms"
            iconPath: "qrc:/nebula/resources/connection-green.svg"
        }
        Rectangle {
            Layout.fillWidth: true

            color: VPNTheme.colors.white
            height: 1
            opacity: 0.2
        }
        VPNConnectionInfoItem {
            title: "Download"
            subtitle: "300.06 Mbps"
            iconPath: "qrc:/nebula/resources/download.svg"
        }
        Rectangle {
            Layout.fillWidth: true

            color: VPNTheme.colors.white
            height: 1
            opacity: 0.2
        }
        VPNConnectionInfoItem {
            title: "Upload"
            subtitle: "21.60 Mbps"
            iconPath: "qrc:/nebula/resources/upload.svg"
        }

        // Rectangle {
        //     color: "gray"

        //     anchors.fill: parent
        //     border.color: "red"
        //     border.width: 2
        //     z: -1
        // }

    }

}