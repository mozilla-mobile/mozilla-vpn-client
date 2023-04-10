/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1
import telemetry 0.30

MZFlickable {
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
                spacing: MZTheme.theme.listSpacing * 0.5

                // Lottie animation
                Item {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.preferredHeight: root.height * 0.25

                    MZLottieAnimation {
                        id: speedometerAnimation
                        loop: false
                        source: ":/ui/resources/animations/speedometer_animation.json"
                    }
                }

                ConnectionCheckmarkList {
                    id: checkmarkList

                    listHeader: VPNConnectionBenchmark.speed === VPNConnectionBenchmark.SpeedSlow
                        ? MZI18n.ConnectionInfoListHeaderSlow
                        : MZI18n.ConnectionInfoListHeaderDefault
                    listModel: checkmarkListModel

                    Layout.bottomMargin: MZTheme.theme.vSpacingSmall
                    Layout.topMargin: MZTheme.theme.listSpacing * 0.5
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin
                }

                ColumnLayout {
                    property bool isMultipHop: (typeof(VPNCurrentServer.entryCountryCode) !== undefined
                        && VPNCurrentServer.entryCountryCode !== "")
                    id: serverLocations

                    spacing: 0

                    Layout.fillWidth: true
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin

                    RowLayout {
                        ConnectionInfoItem {
                            id: entryServerLabel
                            title: serverLocations.isMultipHop
                                ? VPNCurrentServer.localizedEntryCityName
                                : ""
                            subtitle: ""
                            iconPath: serverLocations.isMultipHop
                                ? "qrc:/ui/resources/flags/"
                                    + VPNCurrentServer.entryCountryCode.toUpperCase()
                                    + ".png"
                                : ""
                            isFlagIcon: true
                            visible: serverLocations.isMultipHop
                        }

                        MZIcon {
                            id: arrowIcon
                            source: "qrc:/nebula/resources/arrow-forward-white.svg"
                            sourceSize {
                                height: MZTheme.theme.iconSize * 1.25
                                width: MZTheme.theme.iconSize * 1.25
                            }
                            visible: serverLocations.isMultipHop
                            Layout.fillWidth: true
                            Layout.leftMargin: MZTheme.theme.listSpacing
                            Layout.rightMargin: MZTheme.theme.listSpacing
                        }

                        ConnectionInfoItem {
                            title: serverLocations.isMultipHop
                            ? VPNCurrentServer.localizedExitCityName
                            : VPNCurrentServer.localizedExitCountryName;
                            subtitle: serverLocations.isMultipHop
                                ? ""
                                : VPNCurrentServer.localizedExitCityName
                            iconPath: "qrc:/ui/resources/flags/"
                                + VPNCurrentServer.exitCountryCode.toUpperCase()
                                + ".png"
                            isFlagIcon: true
                        }
                    }

                    Rectangle {
                        color: MZTheme.colors.white
                        height: 1
                        opacity: 0.2
                        Layout.fillWidth: true
                    }

                    ConnectionInfoItem {
                        title: MZI18n.ConnectionInfoLabelPing
                        subtitle: VPNConnectionBenchmark.pingLatency + " " + MZI18n.ConnectionInfoUnitPing
                        iconPath: "qrc:/ui/resources/connection-green.svg"
                    }

                    Rectangle {
                        color: MZTheme.colors.white
                        height: 1
                        opacity: 0.2
                        Layout.fillWidth: true
                    }

                    ConnectionInfoItem {
                        //% "Download"
                        title: qsTrId("vpn.connectionInfo.download")
                        subtitle: root.getConnectionLabel(VPNConnectionBenchmark.downloadBps)
                        iconPath: "qrc:/ui/resources/download.svg"
                    }

                    Rectangle {
                        color: MZTheme.colors.white
                        height: 1
                        opacity: 0.2
                        visible: MZFeatureList.get("benchmarkUpload").isSupported

                        Layout.fillWidth: true
                    }

                    ConnectionInfoItem {
                        title: MZI18n.ConnectionInfoLabelUpload
                        subtitle: root.getConnectionLabel(VPNConnectionBenchmark.uploadBps)
                        iconPath: "qrc:/ui/resources/upload.svg"
                        visible: MZFeatureList.get("benchmarkUpload").isSupported
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
            return MZI18n.ConnectionInfoLabelBitps;
        }

        if (connectionValueBits < Math.pow(1000, 2)) {
            // kbit/s
            return MZI18n.ConnectionInfoLabelKbitps;
        }

        if (connectionValueBits < Math.pow(1000, 3)) {
            // Mbit/s
            return MZI18n.ConnectionInfoLabelMbitps;
        }

        if (connectionValueBits < Math.pow(1000, 4)) {
            // Gbit/s
            return MZI18n.ConnectionInfoLabelGbitps;
        }

        // Tbit/s
        return MZI18n.ConnectionInfoLabelTbitps;
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
                title: MZI18n.ConnectionInfoHighBulletOne,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoHighBulletTwo,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoHighBulletThree,
                type: "checkmark"
            });
        } else if (VPNConnectionBenchmark.speed === VPNConnectionBenchmark.SpeedMedium) {
            // Medium connection threshold
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoMediumBulletOne,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoMediumBulletTwo,
                type: "checkmark"
            });
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoMediumBulletThree,
                type: "checkmark"
            });
        } else {
            // Slow connection threshold
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoTroubleshootingBulletOne,
                type: "arrow",
            });
            checkmarkListModel.append({
                title: MZI18n.ConnectionInfoTroubleshootingBulletTwo,
                type: "arrow",
            });

            if (serverLocations.isMultipHop) {
                checkmarkListModel.append({
                    title: MZI18n.ConnectionInfoTroubleshootingBulletThree,
                    type: "arrow",
                });
            }
        }
    }

}
