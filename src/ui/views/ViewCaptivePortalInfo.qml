/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable

    flickContentHeight: vpnPanel.height + alertWrapperBackground.height + footerContent.height + (Theme.windowMargin * 4)
    state: "required"

    Item {
        id: spacer1

        height: Math.max(Theme.windowMargin * 2, ( window.safeContentHeight - flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    VPNPanel {
        id: vpnPanel

        property var childRectHeight: vpnPanel.childrenRect.height

        anchors.top: spacer1.bottom
        anchors.topMargin: 80
        width: Math.min(vpnFlickable.width - Theme.windowMargin * 2, Theme.maxHorizontalContentWidth)
        logoSize: 80

        logoTitle: VPNl18n.CaptivePortalAlertTitle

        logoSubtitle: VPNl18n.CaptivePortalAlertHeader
        logo: "../resources/globe-warning.svg"
    }

    Item {
        id: spacer2

        anchors.top: vpnPanel.bottom
        height: Math.max(Theme.windowMargin * 2, (window.safeContentHeight -flickContentHeight ) / 2)
        width: vpnFlickable.width
    }

    VPNSubtitle {
            id: subTextBlock
            anchors.top: spacer2.bottom
            anchors.topMargin: Theme.windowMargin
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(vpnFlickable.width - (Theme.windowMargin * 4), Theme.maxHorizontalContentWidth)
            font.family: Theme.fontBoldFamily
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.fontColor
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            text: VPNl18n.CaptivePortalAlertAction
    }



    ColumnLayout {
        id: footerContent

        anchors.top: subTextBlock.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, Theme.maxHorizontalContentWidth)
        spacing: Theme.windowMargin * 1.25

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.windowMargin / 2
            color: "transparent"
        }

        VPNButton {
            id: openPortalButton
            text: VPNl18n.CaptivePortalAlertButtonText
            radius: 4
            onClicked: {
                VPN.openLink(VPN.LinkCaptivePortal);
                stackview.pop()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.windowMargin * 2
            color: "transparent"
        }

    }

}
