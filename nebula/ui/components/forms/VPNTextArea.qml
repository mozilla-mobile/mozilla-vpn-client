/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import components 0.1
import themes 0.1

Item {
    property alias placeholderText: formattedPlaceholderText.text
    property alias textInput: textArea
    property var userEntry: textArea.text
    property bool enabled: true

    id: root

    Layout.preferredHeight: Theme.rowHeight * 3
    Layout.preferredWidth: parent.width
    Layout.maximumHeight: Theme.rowHeight * 3
    Layout.minimumHeight: Theme.rowHeight * 3

    Flickable {
        id: flickable

        anchors.fill: parent
        contentWidth: width
        contentHeight: textArea.implicitHeight

        ScrollBar.vertical: ScrollBar {
            policy: flickable.contentHeight > root.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
            Accessible.ignored: true
        }

        TextArea.flickable: TextArea {
            property int maxCharacterCount: 1000
            property int textLength: textArea.text.length
            property bool forceBlurOnOutsidePress: true
            property bool hasError: false
            property bool showInteractionStates: true

            id: textArea
            clip: true
            textFormat: Text.PlainText
            font.pixelSize: Theme.fontSizeSmall
            font.family: Theme.fontInterFamily
            color: Color.input.default.text
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textMargin: Theme.windowMargin * .75
            padding: 0
            Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            onTextChanged: handleOnTextChanged(text)
            selectByMouse: true
            selectionColor: Theme.input.highlight
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
            enabled: root.enabled
            background: VPNInputBackground {
                itemToFocus: textArea
                z: -1
            }

            VPNTextBlock {
                id: formattedPlaceholderText
                anchors.fill: textArea
                anchors.leftMargin: Theme.windowMargin
                anchors.rightMargin: Theme.windowMargin
                anchors.topMargin: Theme.windowMargin * .75
                color: textAreaStates.state === "emptyHovered" ? Color.input.hover.placeholder : Color.input.default.placeholder
                visible: textArea.text.length < 1

                PropertyAnimation on opacity {
                    duration: 100
                }
            }

            VPNInputStates {
                id: textAreaStates
                itemToTarget: textArea
            }

            function removeCharsInRange(string, startIndex, endIndex) {
                return string.slice(0, startIndex) + string.slice(endIndex);
            }

            function handleOnTextChanged(text) {
                const updatedTextLength = text.length;
                const exceedsCharLimit = updatedTextLength > textArea.maxCharacterCount;

                // Remove most recently added characters in case
                // the text input exceeds the maximum length
                if (exceedsCharLimit) {
                    const textInputLength = updatedTextLength - textLength;
                    // Remember cursor position
                    const prevCursorPosition = textArea.cursorPosition - textInputLength;
                    // Strip overflowing chars
                    const strippedString = removeCharsInRange(text, prevCursorPosition, prevCursorPosition + textInputLength);
                    textArea.text = strippedString;
                    // Restore previous cursor position
                    textArea.cursorPosition = prevCursorPosition;
                }
            }
        }
    }

    Text {
        anchors.top: parent.bottom
        anchors.topMargin: 10
        text: textArea.length + " / " + textArea.maxCharacterCount
        font.pixelSize: 13
        anchors.rightMargin: 8
        anchors.right: parent.right
        color: Theme.fontColor
    }

}

