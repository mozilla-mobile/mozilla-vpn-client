/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14

import Mozilla.VPN 1.0

VPNFlickable {
    id: root

    anchors.fill: parent
    flickContentHeight: col.implicitHeight
    interactive: flickContentHeight > height || contentY > 0

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
        id: col
        spacing: 0

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        ColumnLayout {
            // The ColumnLayout nesting here is a hack to get around binding loops caused by less verbose methods of stipulating
            // that the child content of `root` is vertically centered within `root.height` or is scrollable when root.contentHeight > root.height.
            Layout.minimumHeight: root.height

            ColumnLayout {
                id: viewContent

                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: parent.width
                spacing: VPNTheme.theme.listSpacing * 0.5
                // IP Adresses
                VPNIPAddress {
                    //% "IPv4:"
                    //: The abbreviation for Internet Protocol. This is followed by the user’s IPv4 address.
                    property var ipv4label: qsTrId("vpn.connectionInfo.ipv4")
                    //% "IP:"
                    //: The abbreviation for Internet Protocol. This is followed by the user’s IP address.
                    property var iplabel: qsTrId("vpn.connectionInfo.ip2")

                    ipVersionText: VPNIPAddressLookup.ipv6Address === "" ? iplabel : ipv4label;
                    ipAddressText: VPNIPAddressLookup.ipv4Address
                    visible: VPNIPAddressLookup.ipv4Address !== ""

                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: VPNTheme.theme.windowMargin * 1.5
                }

                VPNIPAddress {
                    visible: VPNIPAddressLookup.ipv6Address !== ""
                    //% "IPv6:"
                    //: The abbreviation for Internet Procol version 6. This is followed by the user’s IPv6 address.
                    ipVersionText: qsTrId("vpn.connectionInfo.ipv6")
                    ipAddressText: VPNIPAddressLookup.ipv6Address
                    Layout.alignment: Qt.AlignHCenter
                }

                // Lottie animation
                Item {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight:  root.height * 0.35

                    VPNLottieAnimation {
                        id: speedometerAnimation
                        loop: false
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
                        subtitle: VPNConnectionBenchmark.pingLatency + " " + VPNl18n.ConnectionInfoUnitPing
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
                        subtitle: root.getConnectionLabel(VPNConnectionBenchmark.downloadBps)
                        iconPath: "qrc:/nebula/resources/download.svg"
                    }

                    Rectangle {
                        color: VPNTheme.colors.white
                        height: 1
                        opacity: 0.2
                        visible: VPNFeatureList.get("benchmarkUpload").isSupported

                        Layout.fillWidth: true
                    }

                    VPNConnectionInfoItem {
                        title: VPNl18n.ConnectionInfoLabelUpload
                        subtitle: root.getConnectionLabel(VPNConnectionBenchmark.uploadBps)
                        iconPath: "qrc:/nebula/resources/upload.svg"
                        visible: VPNFeatureList.get("benchmarkUpload").isSupported
                    }
                }
            }
        }
    }

    function getConnectionLabel(connectionValueBits) {
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
