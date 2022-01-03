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

    state: "required"

    Item {
        id: spacer1
        height: Math.max(VPNTheme.theme.windowMargin * 2, ( window.safeContentHeight - flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    VPNPanel {
        id: vpnPanel
        property var childRectHeight: vpnPanel.childrenRect.height

        anchors.top: spacer1.bottom
        anchors.topMargin: 80
        width: Math.min(vpnFlickable.width - VPNTheme.theme.windowMargin * 2, VPNTheme.theme.maxHorizontalContentWidth)
        logoSize: 80
        logoTitle: VPNl18n.CaptivePortalAlertTitle
        logoSubtitle: VPNl18n.CaptivePortalAlertHeader
        logo: "qrc:/ui/resources/globe-warning.svg"
    }

    Item {
        id: spacer2
        anchors.top: vpnPanel.bottom
        height: Math.max(VPNTheme.theme.windowMargin * 2, (window.safeContentHeight - flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    VPNSubtitle {
        id: subTextBlock
        anchors.top:  vpnPanel.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(vpnFlickable.width - (VPNTheme.theme.windowMargin * 4), VPNTheme.theme.maxHorizontalContentWidth)
        font.family: VPNTheme.theme.fontBoldFamily
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        color: VPNTheme.theme.fontColor
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        text: VPNl18n.CaptivePortalAlertAction
    }



    ColumnLayout {
        id: footerContent

        anchors.top: subTextBlock.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, VPNTheme.theme.maxHorizontalContentWidth)
        spacing: VPNTheme.theme.windowMargin * 1.25

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: VPNTheme.theme.windowMargin / 2
        }

        VPNButton {
            id: openPortalButton
            objectName: "openCaptivePortalButton"
            text: VPNl18n.CaptivePortalAlertButtonText
            radius: 4
            onClicked: {
                VPN.openLink(VPN.LinkCaptivePortal);
                stackview.pop();
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: VPNTheme.theme.windowMargin * 2
        }
    }
}
