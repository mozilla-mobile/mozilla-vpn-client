/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

RowLayout {
    property real maxPaintedTextWidth: box.width - MZTheme.theme.windowMargin
                                       - (stackAddress ? 0 : ipVersion.paintedWidth)
                                       - (showCopyButton ? MZTheme.theme.iconSize * 2 : 0)
    property alias ipVersionText: ipVersion.text
    property alias ipAddressText: ipAddress.text
    property bool showRotateButton: true
    // Detailed rows offer a copy button
    property bool showCopyButton: false
    // Detailed rows have long labels so we stack the ip under the label
    property bool stackAddress: false

    spacing: MZTheme.theme.listSpacing
    Layout.fillWidth: true

    GridLayout {
        columns: stackAddress ? 1 : 2
        columnSpacing: MZTheme.theme.listSpacing
        rowSpacing: 0

        MZBoldInterLabel {
            id: ipVersion

            color: MZTheme.colors.fontColorInverted
            font.pixelSize: MZTheme.theme.fontSizeSmall
            lineHeight: MZTheme.theme.labelLineHeight * (stackAddress ? 1 : 1.25)
            verticalAlignment: Text.AlignVCenter
        }

        MZInterLabel {
            id: ipAddress

            color: MZTheme.colors.fontColorInverted
            font.pixelSize: MZTheme.theme.fontSizeSmall
            horizontalAlignment: Text.AlignLeft
            lineHeight: MZTheme.theme.labelLineHeight * (stackAddress ? 1 : 1.25)
            opacity: 0.8

            Layout.maximumWidth: maxPaintedTextWidth
        }
    }

    // spacer
    Item {
        Layout.fillWidth: true
    }

    MZIconButton {
        id: ipRefreshToggleButton
        visible: showRotateButton && MZFeatureList.get("showRotateIPAddressButton").isSupported && VPNIPAddressLookup.isFinished
        accessibleName: MZI18n.ConnectionInfoRotateIPAddress
        buttonColorScheme: MZTheme.colors.iconButtonDarkBackground
        onClicked: {
            VPN.silentSwitch();
        }

        Image {
            property int iconSize: MZTheme.theme.iconSize * 1.5

            anchors.centerIn: ipRefreshToggleButton
            source: MZAssetLookup.getImageSource("RefreshArrowsIPInfo")
            sourceSize {
                height: iconSize
                width: iconSize
            }
        }
    }

    MZIconButton {
        id: ipCopyButton

        height: MZTheme.theme.iconSize * 2
        width: MZTheme.theme.iconSize * 2
        visible: showCopyButton
        accessibleName: MZI18n.ConnectionInfoCopyEndpointAddress.arg(ipAddress.text)
        buttonColorScheme: MZTheme.colors.iconButtonDarkBackground
        onClicked: {
            MZUtils.storeInClipboard(ipAddress.text);
            MZErrorHandler.requestAlert(MZErrorHandler.CopiedToClipboardConfirmationAlert);
        }

        Image {
            property int iconSize: MZTheme.theme.iconSize * 1.25

            anchors.centerIn: ipCopyButton
            source: MZAssetLookup.getImageSource("CopyIPInfo")
            sourceSize {
                height: iconSize
                width: iconSize
            }
        }
    }
}
