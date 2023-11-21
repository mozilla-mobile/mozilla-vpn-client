/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

ColumnLayout {
    Layout.alignment: Qt.AlignHCenter
    property string _telemetryScreenId

    Text {
        text: MZI18n.InAppAuthTermsOfServiceAndPrivacyDisclaimer
        font.family: MZTheme.theme.fontInterFamily
        font.pixelSize: MZTheme.theme.fontSizeSmall
        color: MZTheme.theme.fontColor
        Layout.fillWidth: true
        Layout.maximumWidth: MZTheme.theme.maxTextWidth
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        horizontalAlignment: Text.AlignHCenter
        linkColor: MZTheme.theme.fontColorDark
        lineHeightMode: Text.FixedHeight
        lineHeight: MZTheme.theme.labelLineHeight
        onLinkActivated: {
            if (link === "terms-of-service") {
                Glean.interaction.termsOfServiceSelected.record({
                    screen: _telemetryScreenId,
                });

                return MZUrlOpener.openUrlLabel("termsOfService");
            }

            Glean.interaction.privacyNoticeSelected.record({
                screen: _telemetryScreenId,
            });

            MZUrlOpener.openUrlLabel("privacyNotice");
        }

        Accessible.role: Accessible.StaticText
        //prevent html tags from being read by screen readers
        //NOTE: This is not a robust way of removing html tags,
        //and should only be used for this particular case
        Accessible.name: text.replace(/<[^>]*>/g, "")
        Accessible.ignored: !visible
    }
}
