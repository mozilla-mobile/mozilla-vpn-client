/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

ColumnLayout {
    Layout.alignment: Qt.AlignHCenter

    Text {
        text: VPNl18n.InAppAuthTermsOfServiceAndPrivacyDisclaimer
        font.family: VPNTheme.theme.fontInterFamily
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        color: VPNTheme.theme.fontColor
        Layout.fillWidth: true
        Layout.maximumWidth: VPNTheme.theme.maxTextWidth
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
        linkColor: VPNTheme.theme.fontColorDark
        lineHeightMode: Text.FixedHeight
        lineHeight: VPNTheme.theme.labelLineHeight
        onLinkActivated: {
            if (link === "terms-of-service")
                return VPN.openLink(VPN.LinkTermsOfService);
            VPN.openLink(VPN.LinkPrivacyNotice);
        }
    }
}
