/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZFlickable {
    id: vpnFlickable

    height: parent.height
    flickContentHeight: column.height

    Component.onCompleted: {
        fade.start();
    }

    ColumnLayout {
        id: column

        spacing: MZTheme.theme.windowMargin * 1.25
        width: vpnFlickable.width

        MZVerticalSpacer {
            Layout.fillWidth: true
            height: MZTheme.theme.windowMargin
        }

        MZPanel {
            logo: "qrc:/ui/resources/updateRecommended.svg"
            logoTitle: MZI18n.TelemetryPolicyViewDataCollectionAndUse
            logoSubtitle: MZI18n.TelemetryPolicyViewDescription
            Layout.fillWidth: true
            anchors.horizontalCenter: undefined
            height: undefined
            width: undefined

            Layout.preferredHeight: childrenRect.height
        }

        ColumnLayout {
            spacing: MZTheme.theme.windowMargin * 1.25
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter

            MZSubtitle {
                id: logoSubtitle

                Layout.alignment: Qt.AlignHCenter
                Layout.leftMargin: MZTheme.theme.windowMargin * 3
                Layout.rightMargin: MZTheme.theme.windowMargin * 3
                Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
                Layout.fillWidth: true
                text: MZI18n.TelemetryPolicyViewQuestion
            }

            MZButton {
                id: button
                objectName: "telemetryPolicyButton"
                //% "Allow on this device"
                text: qsTrId("vpn.telemetryPolicy.allowOnThisDevice")
                radius: 5
                onClicked: {
                    MZSettings.gleanEnabled = true;
                    VPN.telemetryPolicyCompleted();
                }
            }

            MZLinkButton {
                id: linkBtn
                objectName: "declineTelemetryLink"
                //% "Don’t allow"
                labelText: qsTrId("vpn.telemetryPolicy.doNotAllow")
                Layout.alignment: Qt.AlignHCenter
                onClicked: {
                    MZSettings.gleanEnabled = false;
                    VPN.telemetryPolicyCompleted();
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: 0
            MZSubtitle {
                Layout.alignment: Qt.AlignHCenter
                Layout.leftMargin: MZTheme.theme.windowMargin * 3
                Layout.rightMargin: MZTheme.theme.windowMargin * 3
                Layout.maximumWidth: MZTheme.theme.maxHorizontalContentWidth
                Layout.fillWidth: true
                //% "Learn more about what data Mozilla collects and how it’s used."
                text: qsTrId("vpn.telemetryPolicy.learnMoreAboutData")
            }

            MZLinkButton {
                objectName: "privacyLink"
                labelText: MZI18n.InAppSupportWorkflowPrivacyNoticeLinkText
                Layout.alignment: Qt.AlignHCenter
                onClicked: MZUrlOpener.openUrlLabel("privacyNotice")
            }

        }

        MZVerticalSpacer {
            Layout.fillWidth: true
            height: MZTheme.theme.windowMargin * 2
        }
    }

    PropertyAnimation on opacity {
        id: fade

        from: 0
        to: 1
        duration: 300
    }

}
