/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }
    width: parent.width - VPNTheme.theme.windowMargin * 2

    Image {
        Layout.fillWidth: true

        source: "qrc:/ui/resources/connection-error-unstable.svg"
        fillMode: Image.PreserveAspectFit
    }

    VPNBoldLabel {
        Layout.fillWidth: true
        Layout.leftMargin: VPNTheme.theme.vSpacingSmall
        Layout.rightMargin: VPNTheme.theme.windowMargin

        color: VPNTheme.colors.white
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: VPNTheme.theme.fontSizeLarge
        text: VPNl18n.GenericErrorUnexpected
        width: parent.width - VPNTheme.theme.windowMargin
    }

    VPNTextBlock {
        Layout.fillWidth: true
        Layout.bottomMargin: VPNTheme.theme.vSpacingSmall
        Layout.leftMargin: VPNTheme.theme.vSpacingSmall
        Layout.rightMargin: VPNTheme.theme.windowMargin
        Layout.topMargin: VPNTheme.theme.windowMargin

        color: VPNTheme.colors.grey20
        horizontalAlignment: Text.AlignHCenter
        text: VPNl18n.ConnectionInfoErrorMessage
        wrapMode: Text.WordWrap
        width: parent.width < VPNTheme.theme.maxTextWidth
            ? parent.width
            : VPNTheme.theme.maxTextWidth
    }

    VPNButton {
        Layout.fillWidth: true

        // Try again
        text: VPNl18n.GenericPurchaseErrorGenericPurchaseErrorButton
        onClicked: {
            VPNConnectionBenchmark.start();
        }
    }
}
