/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "../../themes/themes.js" as Theme

TextArea {
    Layout.preferredHeight: Theme.rowHeight * 3
    Layout.preferredWidth: parent.width
    textFormat: Text.PlainText
    font.pixelSize: Theme.fontSizeSmall
    font.family: Theme.fontInterFamily
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
    topInset: Theme.windowMargin / 2
    bottomInset: topInset
    leftInset: topInset
    rightInset: topInset
    Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)

    VPNInputBackground {
        z: -1
    }

    Text {
        anchors.top: parent.bottom
        anchors.topMargin:8
        text: "TODO: XX/YY"
        font.pixelSize: 11
        anchors.rightMargin: 8
        anchors.right: parent.right
        color: Theme.fontColor
    }
}
