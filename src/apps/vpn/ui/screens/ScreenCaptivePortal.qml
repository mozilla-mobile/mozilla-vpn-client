/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import components 0.1


VPNFlickable {
    id: vpnFlickable

    readonly property bool isMobile: window.fullScreenRequired()

    flickContentHeight: content.implicitHeight

    state: (VPNController.state == VPNController.StateOff) ? "pre-activation" : "post-activation"
    states: [
        State {
            name: "pre-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: VPNI18n.CaptivePortalAlertActionPreActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: VPNI18n.CaptivePortalAlertButtonTextPreActivation
            }
        },
        State {
            name: "post-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: VPNI18n.CaptivePortalAlertActionPostActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: VPNI18n.CaptivePortalAlertButtonTextPostActivation
            }
        }
    ]


    ColumnLayout {
        id: content

        height: vpnFlickable.height
        width: Math.min(vpnFlickable.width, VPNTheme.theme.maxHorizontalContentWidth)
        spacing: 0

        VPNPanel {
            id: vpnPanel

            anchors.horizontalCenter: undefined
            Layout.topMargin: isMobile ? parent.height * 0.20 : parent.height * 0.12

            logo: "qrc:/ui/resources/globe-warning.svg"
            logoSize: 80
            logoSubtitle: VPNI18n.CaptivePortalAlertHeader
            logoTitle: VPNI18n.CaptivePortalAlertTitle
            width: parent.width

        }

        VPNSubtitle {
            id: subTextBlock

            color: VPNTheme.theme.fontColor
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            font.family: VPNTheme.theme.fontBoldFamily
            width: openPortalButton.width

            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        }

        Item {
            Layout.fillHeight: isMobile
        }

        VPNButton {
            id: openPortalButton

            objectName: "captivePortalAlertActionButton"
            radius: 4
            onClicked: {
                if(vpnFlickable.state === "pre-activation"){
                    VPNUrlOpener.openUrlLabel("captivePortal");
                }
                if(vpnFlickable.state === "post-activation"){
                    VPNCaptivePortal.deactivationRequired();
                }
                VPNNavigator.requestScreen(VPNNavigator.ScreenHome);
            }

            Layout.fillWidth: true
            Layout.topMargin: VPNTheme.theme.vSpacing
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
            Layout.bottomMargin: 58
        }

        Item {
            Layout.fillHeight: !isMobile
        }
    }
}
