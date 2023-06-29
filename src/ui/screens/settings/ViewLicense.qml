/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    property alias licenseContent: licenseText.text
    id: license

    _menuTitle: licenseTitle

    _viewContentData: MZTextBlock {
        id: licenseText
        textFormat: Text.MarkdownText
        width: undefined
        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin
        onLinkActivated: link => MZUrlOpener.openUrl(link)
    }
}
