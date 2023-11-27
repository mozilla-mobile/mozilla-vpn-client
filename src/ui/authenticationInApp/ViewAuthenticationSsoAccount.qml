/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.inAppAuth 0.1

MZInAppAuthenticationThirdParty {
    id: authSsoAccount

    _telemetryScreenId: "authentication_sso_account"
    _viewObjectName: "authSsoAccount"
    _imgSource: "qrc:/ui/resources/create-password.svg"
    _headlineText: "Create a password"

    flickContentHeight: col.implicitHeight

    _description: ColumnLayout {
        MZTextBlock {
          id: ssoInstructionLine1
          text: MZI18n.InAppAuthSsoInstructionLine1

          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: 22
          Layout.fillWidth: true
          Layout.fillHeight: true
        }

        MZTextBlock {
          id: ssoInstructionLine2
          text: MZI18n.InAppAuthSsoInstructionLine2

          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.leftMargin: MZTheme.theme.vSpacing / 2
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: 22

          // TODO link styling
          onLinkActivated: link => MZUrlOpener.openUrlLabel("setSsoPassword")
        }

        MZTextBlock {
          id: ssoInstructionLine3
          text: MZI18n.InAppAuthSsoInstructionLine3

          Layout.fillWidth: true
          Layout.fillHeight: true
          Layout.leftMargin: MZTheme.theme.vSpacing / 2
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: 22
        }

        MZTextBlock {
          id: ssoInstructionLine4
          text: MZI18n.InAppAuthSsoInstructionLine4

          Layout.fillWidth: true
          Layout.fillHeight: true
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: 22
        }

    }
}
