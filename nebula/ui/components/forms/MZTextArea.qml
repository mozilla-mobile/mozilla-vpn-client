/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

Item {
    property alias placeholderText: formattedPlaceholderText.text
    property alias textInput: textArea
    property var userEntry: textArea.text
    property bool enabled: true

    id: root

    Layout.maximumHeight: MZTheme.theme.rowHeight * 3
    Layout.minimumHeight: MZTheme.theme.rowHeight * 3
    Layout.preferredHeight: MZTheme.theme.rowHeight * 3
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
            background: MZInputBackground {
                itemToFocus: textArea
                z: -1
            }
            bottomPadding: MZTheme.theme.windowMargin
            clip: true
            color: MZTheme.colors.input.default.text
            cursorDelegate: MZCursorDelegate {}
            enabled: root.enabled
            font.family: MZTheme.theme.fontInterFamily
            font.pixelSize: MZTheme.theme.fontSizeSmall
            inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
            leftPadding: MZTheme.theme.windowMargin
            rightPadding: MZTheme.theme.windowMargin
            selectByMouse: true
            selectionColor: MZTheme.theme.input.highlight
            textFormat: Text.PlainText
            topPadding: MZTheme.theme.windowMargin
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            Keys.onTabPressed: nextItemInFocusChain().forceActiveFocus(Qt.TabFocusReason)
            onTextChanged: {
                handleOnTextChanged(textArea.text);

                // This is a workaround for VoiceOver on macOS: https://bugreports.qt.io/browse/QTBUG-108189
                // After gaining initial focus or typing in TextArea the screen reader
                // fails to narrate any accessible content and action. After regaining
                // active focus the screen reader keeps working as expected.
                if (Qt.platform.os === "osx") {
                    Accessible.focused = false
                    Accessible.focused = true
                }
            }

            onActiveFocusChanged: if(activeFocus)  MZUiUtils.scrollToComponent(textArea)

            Connections {
                target: window
                function onScreenClicked(x, y) {
                    if(textArea.focus) window.removeFocus(textArea, x, y)
                }
            }

            MZTextBlock {
                id: formattedPlaceholderText

                anchors {
                    fill: textArea
                    leftMargin: MZTheme.theme.windowMargin
                    rightMargin: MZTheme.theme.windowMargin
                    topMargin: MZTheme.theme.windowMargin
                }
                color: textAreaStates.state === "emptyHovered"
                    ? MZTheme.colors.input.hover.placeholder
                    : MZTheme.colors.input.default.placeholder
                visible: textArea.text.length < 1

                PropertyAnimation on opacity {
                    duration: 100
                }
            }

            MZInputStates {
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
            topMargin: MZTheme.theme.listSpacing * 1.25
            right: parent.right
            rightMargin: MZTheme.theme.listSpacing
        }
        color: MZTheme.theme.fontColor
        font.pixelSize: MZTheme.theme.fontSizeSmall
        text: textArea.length + " / " + textArea.maxCharacterCount
        Accessible.ignored: !visible
    }

}

