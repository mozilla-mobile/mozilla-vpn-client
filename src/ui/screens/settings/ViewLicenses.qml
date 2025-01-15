/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZViewBase {
    _menuTitle: MZI18n.AboutUsLicenses
    _viewContentData: Column {
        Layout.fillWidth: true

        MZTextBlock {
            id: mainLicense

            anchors.horizontalCenter: parent.horizontalCenter
            text: MZLicenseModel.contentLicense
            textFormat: Text.MarkdownText
            width: parent.width - MZTheme.theme.windowMargin * 2
            onLinkActivated: link => MZUrlOpener.openUrl(link)
        }

        MZVerticalSpacer {
            height: MZTheme.theme.listSpacing * 0.5
        }

        Repeater {
            model: MZLicenseModel

            MZExternalLinkListItem {
                accessibleName: licenseTitle
                title: licenseTitle
                iconSource: MZAssetLookup.getImageSource("Chevron")
                iconMirror: MZLocalizer.isRightToLeft
                onClicked: stackview.push("qrc:/qt/qml/Mozilla/VPN/screens/settings/ViewLicense.qml", { _menuTitle: licenseTitle, licenseContent })
            }
        }
    }

    Component.onCompleted: MZLicenseModel.initialize()
}

