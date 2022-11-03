/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

Item {
    property alias placeholderText: formattedPlaceholderText.text
    property alias textInput: textArea
    property var userEntry: textArea.text
    property bool enabled: true

    id: root

    Layout.maximumHeight: VPNTheme.theme.rowHeight * 3
    Layout.minimumHeight: VPNTheme.theme.rowHeight * 3
    Layout.preferredHeight: VPNTheme.theme.rowHeight * 3
    Layout.preferredWidth: parent.width

    Flickable {
        id: flickable

        anchors.fill: parent
        contentHeight: textArea.implicitHeight
        contentWidth: width

        ScrollBar.vertical: ScrollBar {
            policy: flickable.contentHeight > root.height
                ? ScrollBar.AlwaysOn
                : ScrollBar.AlwaysOff
            Accessible.ignored: true
        }

        TextArea.flickable: TextArea {
            property int maxCharacterCount: 1000
            property int textLength: textArea.text.length
            property bool forceBlurOnOutsidePress: true
            property bool hasError: false
            property bool showInteractionStates: true

            id: textArea

            Accessible.focused: textArea.focus
            background: VPNInputBackground {
                itemToFocus: textArea
                z: -1
            }
            bottomPadding: VPNTheme.theme.windowMargin
            clip: true
            color: VPNTheme.colors.input.default.text
            cursorDelegate: VPNCursorDelegate {}
            enabled: root.enabled
            font.family: VPNTheme.theme.fontInterFamily
            font.pixelSize: VPNTheme.theme.fontSizeSmall
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
            leftPadding: VPNTheme.theme.windowMargin
            rightPadding: VPNTheme.theme.windowMargin
            selectByMouse: true
            selectionColor: VPNTheme.theme.input.highlight
            textFormat: Text.PlainText
            topPadding: VPNTheme.theme.windowMargin
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            onTextChanged: {
                handleOnTextChanged(textArea.text);

                // This is a workaround for VoiceOver on macOS: https://bugreports.qt.io/browse/QTBUG-108189
                // After gaining initial focus or typing in TextArea the screen reader
                // fails to narrate any accessible content and action. After regaining
                // active focus the screen reader keeps working as expected.
                if (Qt.platform.os === "osx") {
                    textArea.focus = false;
                    textArea.forceActiveFocus();
                }
            }

            VPNTextBlock {
                id: formattedPlaceholderText

                anchors {
                    fill: textArea
                    leftMargin: VPNTheme.theme.windowMargin
                    rightMargin: VPNTheme.theme.windowMargin
                    topMargin: VPNTheme.theme.windowMargin
                }
                color: textAreaStates.state === "emptyHovered"
                    ? VPNTheme.colors.input.hover.placeholder
                    : VPNTheme.colors.input.default.placeholder
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
                    const strippedString = removeCharsInRange(
                        text,
                        prevCursorPosition,
                        prevCursorPosition + textInputLength
                    );
                    textArea.text = strippedString;
                    // Restore previous cursor position
                    textArea.cursorPosition = prevCursorPosition;
                }
            }
        }
    }

    Text {
        anchors {
            top: parent.bottom
            topMargin: VPNTheme.theme.listSpacing * 1.25
            right: parent.right
            rightMargin: VPNTheme.theme.listSpacing
        }
        color: VPNTheme.theme.fontColor
        font.pixelSize: VPNTheme.theme.fontSizeSmall
        text: textArea.length + " / " + textArea.maxCharacterCount
    }

}

