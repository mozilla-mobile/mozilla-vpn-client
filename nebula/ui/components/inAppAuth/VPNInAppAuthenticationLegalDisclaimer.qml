/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

ColumnLayout {
    Layout.alignment: Qt.AlignHCenter

    Text {
        text: VPNI18n.InAppAuthTermsOfServiceAndPrivacyDisclaimer
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
            if (link === "terms-of-service") {
                return VPNUrlOpener.openUrlLabel("termsOfService");
            }
            VPNUrlOpener.openUrlLabel("privacyNotice");
        }

        Accessible.role: Accessible.StaticText
        //prevent html tags from being read by screen readers
        //NOTE: This is not a robust way of removing html tags,
        //and should only be used for this particular case
        Accessible.name: text.replace(/<[^>]*>/g, "")
    }
}
