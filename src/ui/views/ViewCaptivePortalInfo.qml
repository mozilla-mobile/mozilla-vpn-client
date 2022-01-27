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

    flickContentHeight: content.height

    state: (VPNController.state == VPNController.StateOff) ? "pre-activation" : "post-activation"
    states: [
        State {
            name: "pre-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: VPNl18n.CaptivePortalAlertActionPreActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: VPNl18n.CaptivePortalAlertButtonTextPreActivation
            }
        },
        State {
            name: "post-activation"
            PropertyChanges { 
                target: subTextBlock; 
                text: VPNl18n.CaptivePortalAlertActionPostActivation
            }
            PropertyChanges{
                target:openPortalButton
                text: VPNl18n.CaptivePortalAlertButtonTextPostActivation
            }
        }
    ]


    ColumnLayout {
        id: content

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        width: Math.min(vpnFlickable.width, VPNTheme.theme.maxHorizontalContentWidth)

        VPNPanel {
            id: vpnPanel
            property var childRectHeight: vpnPanel.childrenRect.height

            logo: "qrc:/ui/resources/globe-warning.svg"
            logoSize: 80
            logoSubtitle: VPNl18n.CaptivePortalAlertHeader
            logoTitle: VPNl18n.CaptivePortalAlertTitle
            width: parent.width

            anchors.horizontalCenter: undefined
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
        }

        VPNSubtitle {
            id: subTextBlock

            color: VPNTheme.theme.fontColor
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            font.family: VPNTheme.theme.fontBoldFamily
            text: VPNl18n.CaptivePortalAlertPreActivation
            width: openPortalButton.width

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
            Layout.topMargin: VPNTheme.theme.vSpacingSmall
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        }

        VPNButton {
            id: openPortalButton

            objectName: "captivePortalAlertActionButton"
            text: VPNl18n.CaptivePortalAlertPreActivation
            radius: 4
            onClicked: {
                if(vpnFlickable.state === "pre-activation"){
                    VPN.openLink(VPN.LinkCaptivePortal);
                }
                if(vpnFlickable.state === "post-activation"){
                    VPNCaptivePortal.deactivationRequired();
                }
                stackview.pop();
            }

            Layout.fillWidth: true
            Layout.leftMargin: VPNTheme.theme.windowMargin * 2
            Layout.rightMargin: VPNTheme.theme.windowMargin * 2
        }

    }
}
