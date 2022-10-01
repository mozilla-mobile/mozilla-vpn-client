/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1


VPNViewBase {
    _menuTitle: VPNl18n.AboutUsLicenses
    _viewContentData: Column {
        Layout.fillWidth: true

        VPNTextBlock {
            id: mainLicense

            anchors.horizontalCenter: parent.horizontalCenter
            text: VPNLicenseModel.contentLicense
            textFormat: Text.MarkdownText
            width: parent.width - VPNTheme.theme.windowMargin * 2
            onLinkActivated: link => VPNUrlOpener.openUrl(link)
        }

        VPNVerticalSpacer {
            height: VPNTheme.theme.listSpacing * 0.5
        }

        Repeater {
            model: VPNLicenseModel

            VPNExternalLinkListItem {
                accessibleName: licenseTitle
                title: licenseTitle
                iconSource: "qrc:/nebula/resources/chevron.svg"
                onClicked: stackview.push("qrc:/ui/screens/settings/ViewLicense.qml", { _menuTitle: licenseTitle, licenseContent })
            }
        }
    }

    Component.onCompleted: VPNLicenseModel.initialize()
}

