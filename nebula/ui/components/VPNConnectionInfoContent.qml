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
            populateCheckmarkListModel();
        } else {
            speedometerAnimation.stop();
            checkmarkListModel.clear();
        }
    }

    ListModel {
        id: checkmarkListModel
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
            id: checkmarkList

            listHeader: VPNConnectionBenchmark.speed === VPNConnectionBenchmark.SpeedSlow
                ? VPNl18n.ConnectionInfoListHeaderSlow
                : VPNl18n.ConnectionInfoListHeaderDefault
            listModel: checkmarkListModel

            Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
            Layout.topMargin: VPNTheme.theme.listSpacing * 0.5
            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin
        }

        ColumnLayout {
            spacing: 0

            Layout.leftMargin: VPNTheme.theme.windowMargin
            Layout.rightMargin: VPNTheme.theme.windowMargin

            VPNConnectionInfoItem {
                title: VPNServerCountryModel.getLocalizedCountryName(
                    VPNCurrentServer.exitCountryCode
                )
                subtitle: VPNCurrentServer.localizedCityName
                iconPath: "qrc:/nebula/resources/flags/"
                    + VPNCurrentServer.exitCountryCode.toUpperCase()
                    + ".png"
                isFlagIcon: true
            }

            Rectangle {
                color: VPNTheme.colors.white
                height: 1
                opacity: 0.2

                Layout.fillWidth: true
            }

            VPNConnectionInfoItem {
                title: VPNl18n.ConnectionInfoLabelPing
                subtitle: VPNConnectionBenchmark.ping + " " + VPNl18n.ConnectionInfoUnitPing
                iconPath: "qrc:/nebula/resources/connection-green.svg"
            }

            Rectangle {
                color: VPNTheme.colors.white
                height: 1
                opacity: 0.2

                Layout.fillWidth: true
            }

            VPNConnectionInfoItem {
                //% "Download"
                title: qsTrId("vpn.connectionInfo.download")
                subtitle: root.getConnectionLabel(VPNConnectionBenchmark.download)
                iconPath: "qrc:/nebula/resources/download.svg"
            }

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

    function populateCheckmarkListModel() {
        // Fast connection threshold
        if (VPNConnectionBenchmark.speed === VPNConnectionBenchmark.SpeedFast) {
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoHighBulletOne,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoHighBulletTwo,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoHighBulletThree,
                type: "checkmark"
            });
        } else if (VPNConnectionBenchmark.speed === VPNConnectionBenchmark.SpeedMedium) {
            // Medium connection threshold
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoMediumBulletOne,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoMediumBulletTwo,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoMediumBulletThree,
                type: "checkmark"
            });
        } else {
            // Slow connection threshold
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoTroubleshootingBulletOne,
                type: "arrow",
            });
            checkmarkListModel.append({
                title: VPNl18n.ConnectionInfoTroubleshootingBulletTwo,
                type: "arrow",
            });
        }
    }

}
