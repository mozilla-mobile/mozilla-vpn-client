import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Flickable {
    id: root

    property var rBytes: VPNConnectionData.rxBytes
    property var tBytes: VPNConnectionData.txBytes

    height: parent.height
    contentHeight: Math.max(content.height, height)
    width: parent.width
    onVisibleChanged: {
        if (visible) {
            speedometerAnimation.play();
        } else {
            speedometerAnimation.stop();
        }
    }

    ColumnLayout {
        id: content
        
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0
        width: parent.width - VPNTheme.theme.windowMargin * 2

        // IP Adresses
        VPNIPAddress {
            //% "IPv4:"
            //: The abbreviation for Internet Protocol. This is followed by the user’s IPv4 address.
            property var ipv4label: qsTrId("vpn.connectionInfo.ipv4")
            //% "IP:"
            //: The abbreviation for Internet Protocol. This is followed by the user’s IP address.
            property var iplabel: qsTrId("vpn.connectionInfo.ip2")

            ipVersionText: VPNConnectionData.ipv6Address === "" ? iplabel : ipv4label;
            ipAddressText: VPNConnectionData.ipv4Address
            visible: VPNConnectionData.ipv4Address !== ""

            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
            Layout.topMargin: VPNTheme.theme.windowMargin * 1.5
        }

        VPNIPAddress {
            visible: VPNConnectionData.ipv6Address !== ""
            //% "IPv6:"
            //: The abbreviation for Internet Procol version 6. This is followed by the user’s IPv6 address.
            ipVersionText: qsTrId("vpn.connectionInfo.ipv6")
            ipAddressText: VPNConnectionData.ipv6Address

            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: VPNTheme.theme.listSpacing * 0.5
        }

        // Lottie animation
        Item {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.minimumHeight: 75
            Layout.preferredHeight: 100

            VPNLottieAnimation {
                id: speedometerAnimation
                source: ":/nebula/resources/animations/speedometer_animation.json"
            }
        }

        // Bullet list
        VPNTextBlock {
            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.listSpacing * 0.5
            Layout.bottomMargin: VPNTheme.theme.listSpacing

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
            //% "Download"
            //: The current download speed. The speed is shown on the next line.
            title: qsTrId("vpn.connectionInfo.download")
            subtitle: root.rBytes
            iconPath: "qrc:/nebula/resources/download.svg"
        }
        Rectangle {
            Layout.fillWidth: true

            color: VPNTheme.colors.white
            height: 1
            opacity: 0.2
        }
        VPNConnectionInfoItem {
            //% "Upload"
            //: The current upload speed. The speed is shown on the next line.
            title: qsTrId("vpn.connectionInfo.upload")
            subtitle: root.getConnectionLabel(root.tBytes)
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

    function getConnectionLabel(connectionValue) {
        const connectionValueBits = connectionValue * 8; // convert bytes to bits
        return `${computeValue(connectionValueBits)} ${computeRange(connectionValueBits)}`;
    }

    function computeRange(connectionValueBits) {
        if (connectionValueBits < 1000) {
            // bit/s
            return VPNl18n.ConnectionInfoLabelBitps;
        }

        if (connectionValueBits < Math.pow(1000, 2)) {
            // kbit/s
            return VPNl18n.ConnectionInfoLabelKbitps;
        }

        if (connectionValueBits < Math.pow(1000, 3)) {
            // Mbit/s
            return VPNl18n.ConnectionInfoLabelMbitps;
        }

        if (connectionValueBits < Math.pow(1000, 4)) {
            // Gbit/s
            return VPNl18n.ConnectionInfoLabelGbitps;
        }

        // Tbit/s
        return VPNl18n.ConnectionInfoLabelTbitps;
    }

    function roundValue(value) {
        return Math.round(value * 100) / 100;
    }

    function computeValue(connectionValueBits) {
        if (connectionValueBits < 1000)
            return roundValue(connectionValueBits);

        if (connectionValueBits < Math.pow(1000, 2))
            return roundValue(connectionValueBits / 1000);

        if (connectionValueBits < Math.pow(1000, 3))
            return roundValue(connectionValueBits / Math.pow(1000, 2));

        if (connectionValueBits < Math.pow(1000, 4))
            return roundValue(connectionValueBits / Math.pow(1000, 3));

        return roundValue(connectionValueBits / Math.pow(1000, 4));
    }

}
