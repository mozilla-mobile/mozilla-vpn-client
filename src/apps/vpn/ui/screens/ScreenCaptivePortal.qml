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

    readonly property bool isMobile: window.fullScreenRequired()

    flickContentHeight: content.implicitHeight

    state: (VPNController.state == VPNController.StateOff) ? "pre-activation" : "post-activation"
    states: [
        State {
            name: "pre-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: MZI18n.CaptivePortalAlertActionPreActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: MZI18n.CaptivePortalAlertButtonTextPreActivation
            }
        },
        State {
            name: "post-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: MZI18n.CaptivePortalAlertActionPostActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: MZI18n.CaptivePortalAlertButtonTextPostActivation
            }
        }
    ]


    ColumnLayout {
        id: content

        height: vpnFlickable.height
        width: Math.min(vpnFlickable.width, MZTheme.theme.maxHorizontalContentWidth)
        spacing: 0

        MZPanel {
            id: vpnPanel

            anchors.horizontalCenter: undefined
            Layout.topMargin: isMobile ? parent.height * 0.20 : parent.height * 0.12

            logo: "qrc:/ui/resources/globe-warning.svg"
            logoSize: 80
            logoSubtitle: MZI18n.CaptivePortalAlertHeader
            logoTitle: MZI18n.CaptivePortalAlertTitle
            width: parent.width

        }

        MZSubtitle {
            id: subTextBlock

            color: MZTheme.theme.fontColor
            font.pixelSize: MZTheme.theme.fontSizeSmall
            font.family: MZTheme.theme.fontBoldFamily
            width: openPortalButton.width

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacingSmall
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
        }

        Item {
            Layout.fillHeight: isMobile
        }

        MZButton {
            id: openPortalButton

            objectName: "captivePortalAlertActionButton"
            radius: 4
            onClicked: {
                if(vpnFlickable.state === "pre-activation"){
                    MZUrlOpener.openUrlLabel("captivePortal");
                }
                if(vpnFlickable.state === "post-activation"){
                    VPNCaptivePortal.deactivationRequired();
                }
                VPNNavigator.requestScreen(VPNNavigator.ScreenHome);
            }

            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.vSpacing
            Layout.leftMargin: MZTheme.theme.windowMargin * 2
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.bottomMargin: 58
        }

        Item {
            Layout.fillHeight: !isMobile
        }
    }
}
