/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import "../../themes/themes.js" as Theme

Item {
    property alias placeholderText: textArea.placeholderText

    id: root
    Layout.preferredHeight: Theme.rowHeight * 3
    Layout.preferredWidth: parent.width
    Layout.maximumHeight: Theme.rowHeight * 3
    Layout.minimumHeight: Theme.rowHeight * 3


    VPNInputBackground {
        itemToFocus: textArea
        z: -1
    }


    Flickable {
        id: flickable

        anchors.fill: parent
        contentWidth: width
        contentHeight: textArea.implicitHeight
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: flickable.contentHeight > root.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            Accessible.ignored: true
        }

        TextArea.flickable: TextArea {
            property var maxCharacterCount: 1000
            id: textArea
            textFormat: Text.PlainText
            font.pixelSize: Theme.fontSizeSmall
            font.family: Theme.fontInterFamily
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            topInset: Theme.windowMargin / 2
            bottomInset: topInset / 2
            leftInset: topInset / 2
            rightInset: topInset / 2
            Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            onTextChanged: if (length > maxCharacterCount) remove(maxCharacterCount, length)
            selectByMouse: true
            selectionColor: Theme.input.highlight
        }
    }


    Text {
        anchors.top: parent.bottom
        anchors.topMargin:8
        text: textArea.length + " / " + textArea.maxCharacterCount
        font.pixelSize: 13
        anchors.rightMargin: 8
        anchors.right: parent.right
        color: Theme.fontColor
    }
}

