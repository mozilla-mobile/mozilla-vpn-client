/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

Flickable {
    id: root

    contentHeight: Math.max(content.height, height)
    height: parent.height
    width: parent.width
    onVisibleChanged: {
        if (visible) {
            speedometerAnimation.play();
        } else {
            speedometerAnimation.stop();
        }
    }

    // TODO: This list will be dynamic depending on current connection speeds
    ListModel {
        id: checkmarkListModel

        ListElement {
            title: "Streaming in 4K"
        }
        ListElement {
            title: "High-speed downloads"
        }
        ListElement {
            title: "Online gaming"
        }
    }

    // TODO: This list will be dynamic generated depending
    // on the available connection information we have
    ListModel {
        id: connectionInfoModel

        Component.onCompleted: {
            connectionInfoModel.append({
                titleString: VPNServerCountryModel.getLocalizedCountryName(
                    VPNCurrentServer.exitCountryCode
                ),
                subtitleString: VPNCurrentServer.localizedCityName,
                iconSrc: "qrc:/nebula/resources/flags/"
                    + VPNCurrentServer.exitCountryCode.toUpperCase()
                    + ".png",
                isFlag: true
            });
            connectionInfoModel.append({
                titleString: "Ping",
                subtitleString: "15 ms",
                iconSrc: "qrc:/nebula/resources/connection-green.svg"
            });
            connectionInfoModel.append({
                //% "Download"
                //: The current download speed. The speed is shown on the next line.
                titleString: qsTrId("vpn.connectionInfo.download"),
                subtitleString: root.getConnectionLabel(1234567890),
                iconSrc: "qrc:/nebula/resources/download.svg"
            });
            connectionInfoModel.append({
                //% "Upload"
                //: The current upload speed. The speed is shown on the next line.
                titleString: qsTrId("vpn.connectionInfo.upload"),
                subtitleString: root.getConnectionLabel(123456789),
                iconSrc: "qrc:/nebula/resources/upload.svg"
            });
        }
    }

    ColumnLayout {
        id: content
        
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 0
        width: parent.width

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
            Layout.preferredHeight: parent.width * 0.35

            VPNLottieAnimation {
                id: speedometerAnimation
                source: ":/nebula/resources/animations/speedometer_animation.json"
            }
        }

        VPNCheckmarkList {
            // TODO: Replace with localized string
            listHeader: "At your current speed, here's what your device is optimized for:"
            listModel: checkmarkListModel

            Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
            Layout.topMargin: VPNTheme.theme.listSpacing * 0.5
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
        }

        Repeater {
            model: VPNConnectionBenchmarkModel
            delegate: Text {
                color: "white"
                text: benchmark.id + ", " + benchmark.displayName + ", " + benchmark.result
            }
        }

        Component {
            id: connectionInfoItem

            ColumnLayout {
                spacing: 0
                Rectangle {
                    color: VPNTheme.colors.white
                    height: 1
                    opacity: 0.2
                    visible: index !== 0

                    Layout.fillWidth: true
                }

                VPNConnectionInfoItem {
                    title: titleString
                    subtitle: subtitleString
                    iconPath: iconSrc
                    isFlagIcon: isFlag
                }

                Layout.leftMargin: VPNTheme.theme.windowMargin
                Layout.rightMargin: VPNTheme.theme.windowMargin
            }
        }

        Repeater {
            id: connectionInfoList

            model: connectionInfoModel
            delegate: connectionInfoItem
        }

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
