/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

VPNFlickable {
    id: vpnFlickable

    Component.onCompleted: {
        flickContentHeight = column.childrenRect.height;
        fade.start();
    }

    ColumnLayout {
        id: column
        width: vpnFlickable.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        spacing: Theme.windowMargin * 1.25

        VPNVerticalSpacer {
            Layout.fillWidth: true
            height: Theme.windowMargin
        }

        VPNPanel {
            logo: "qrc:/ui/resources/updateRecommended.svg"
            logoTitle: qsTrId("vpn.settings.dataCollection")
            //% "We strive to provide you with choices and collect only the technical data we need to improve Mozilla VPN. Sharing this data with Mozilla is optional."
            logoSubtitle: qsTrId("vpn.telemetryPolicy.updatedTelemetryDisclaimer")
            Layout.fillWidth: true
            anchors.horizontalCenter: undefined
            height: undefined
            width: undefined

            Layout.preferredHeight: childrenRect.height
        }

        ColumnLayout {
            spacing: Theme.windowMargin * 1.25
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            VPNSubtitle {
                id: logoSubtitle

                Layout.alignment: Qt.AlignHCenter
                Layout.leftMargin: Theme.windowMargin * 3
                Layout.rightMargin: Theme.windowMargin * 3
                Layout.maximumWidth: Theme.maxHorizontalContentWidth
                Layout.fillWidth: true
                //% "Allow Mozilla VPN to send technical data to Mozilla?"
                text: qsTrId("vpn.telemetryPolicy.allowMozillaToSendData")
            }

            VPNButton {
                id: button
                objectName: "telemetryPolicyButton"
                //% "Allow on this device"
                text: qsTrId("vpn.telemetryPolicy.allowOnThisDevice")
                radius: 5
                onClicked: {
                    VPNSettings.gleanEnabled = true;
                    VPN.telemetryPolicyCompleted();
                }
            }

            VPNLinkButton {
                id: linkBtn
                objectName: "declineTelemetryLink"
                //% "Don’t allow"
                labelText: qsTrId("vpn.telemetryPolicy.doNotAllow")
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    VPNSettings.gleanEnabled = false;
                    VPN.telemetryPolicyCompleted();
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 0
            VPNSubtitle {
                Layout.alignment: Qt.AlignHCenter
                Layout.leftMargin: Theme.windowMargin * 3
                Layout.rightMargin: Theme.windowMargin * 3
                Layout.maximumWidth: Theme.maxHorizontalContentWidth
                Layout.fillWidth: true
                //% "Learn more about what data Mozilla collects and how it’s used."
                text: qsTrId("vpn.telemetryPolicy.learnMoreAboutData")
            }

            VPNLinkButton {
                objectName: "privacyLink"
                //% "Mozilla VPN Privacy Notice"
                labelText: qsTrId("vpn.telemetryPolicy.MozillaVPNPrivacyNotice")
                Layout.alignment: Qt.AlignHCenter
                onClicked: VPN.openLink(VPN.LinkPrivacyNotice)
            }

        }

        VPNVerticalSpacer {
            Layout.fillWidth: true
            height: Theme.windowMargin * 2
        }
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 300
    }

}
