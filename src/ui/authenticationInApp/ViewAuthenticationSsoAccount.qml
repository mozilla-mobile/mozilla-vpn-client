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
    _headlineText: MZI18n.InAppAuthSsoInstructionHeader

    _description: ColumnLayout {

        MZTextBlock {
          id: ssoInstructionLine1
          text: MZI18n.InAppAuthSsoInstructionLine1

          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: MZTheme.theme.labelLineHeight
          Layout.fillWidth: true
        }

        MZTextBlock {
          id: ssoInstructionLine2
          text: MZI18n.InAppAuthSsoInstructionLine2a

          Layout.fillWidth: true
          Layout.leftMargin: MZTheme.theme.vSpacing / 2
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: MZTheme.theme.labelLineHeight

          onLinkActivated: link => MZUrlOpener.openUrlLabel("setSsoPassword")
        }

        MZTextBlock {
          id: ssoInstructionLine3
          text: MZI18n.InAppAuthSsoInstructionLine3

          Layout.fillWidth: true
          Layout.leftMargin: MZTheme.theme.vSpacing / 2
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: MZTheme.theme.labelLineHeight
        }

        MZTextBlock {
          id: ssoInstructionLine4
          text: MZI18n.InAppAuthSsoInstructionLine4

          Layout.fillWidth: true
          font.pixelSize: MZTheme.theme.fontSize
          lineHeight: MZTheme.theme.labelLineHeight
        }

        MZVerticalSpacer {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: MZTheme.theme.vSpacing / 2
            Layout.maximumHeight: MZTheme.theme.vSpacing * 2
        }

    }
}
