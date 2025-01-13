/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZInAppAuthenticationThirdParty {
    id: authStubAccount
    objectName: "authStubAccount"

    _telemetryScreenId: "authentication_stub_account"
    _imgSource: MZAssetLookup.getImageSource("CheckEmail")
    _headlineText: MZI18n.InAppAuthStubAccountVerificationHeader

    _description: ColumnLayout {
      MZTextBlock {
        id: descriptionText
        text: MZI18n.InAppAuthStubAccountVerificationInstruction.arg("<b style='font-family:" + MZTheme.theme.fontInterSemiBoldFamily + "; font-weight: " + MZTheme.theme.fontWeightBold + ";'>" + MZAuthInApp.emailAddress + "</b>")

        textFormat: Text.RichText
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        Layout.preferredHeight: paintedHeight
        Layout.alignment: Qt.AlignHCenter
        font.pixelSize: MZTheme.theme.fontSize
        lineHeight: MZTheme.theme.labelLineHeight
      }
    }
}
