/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

TextField {
    property bool hasError: false
    property bool showInteractionStates: true
    property bool forceBlurOnOutsidePress: true
    property alias _placeholderText: centeredPlaceholderText.text
    property bool focusReasonA11y: false

    id: textField

    Accessible.name: centeredPlaceholderText.text
    Accessible.description:  centeredPlaceholderText.text
    Accessible.focused: textField.focus
    Layout.alignment: Qt.AlignVCenter
    Layout.preferredHeight: MZTheme.theme.rowHeight

    background: MZInputBackground {
        id: textFieldBackground
    }
    bottomPadding: MZTheme.theme.windowMargin / 2
    color: MZTheme.colors.input.default.text
    cursorDelegate: MZCursorDelegate {}
    echoMode: TextInput.Normal
    font.family: MZTheme.theme.fontInterFamily
    font.pixelSize: MZTheme.theme.fontSizeSmall
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    leftPadding: MZTheme.theme.windowMargin
    placeholderTextColor: MZTheme.colors.grey40
    rightPadding: MZTheme.theme.windowMargin
    selectByMouse: true
    topPadding: MZTheme.theme.windowMargin / 2
    verticalAlignment: TextInput.AlignVCenter
    horizontalAlignment: TextInput.AlignLeft

    onActiveFocusChanged: {
        if (activeFocus) {
            MZUiUtils.scrollToComponent(textField)
        }
    }

    // This is a workaround for VoiceOver on macOS: https://bugreports.qt.io/browse/QTBUG-108189
    // After gaining initial focus or typing in TextField the screen reader
    // fails to narrate any accessible content and action. After regaining
    // active focus the screen reader keeps working as expected.
    onTextChanged: {
        if (Qt.platform.os === "osx") {
            textField.focusReasonA11y = true
            Accessible.focused = false
            Accessible.focused = true
            textField.focusReasonA11y = false
        }
    }

    Text {
        id: centeredPlaceholderText

        Accessible.ignored: true

        color: textField.placeholderTextColor
        elide: Text.ElideRight
        font: textField.font
        height: MZTheme.theme.rowHeight
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        visible: !textField.length && !textField.preeditText
        width: textField.width - (textField.leftPadding + textField.rightPadding)
        x: textField.leftPadding
    }

    MZInputStates {
        id: textFieldState

        itemToTarget: textField
    }

    MouseArea {
        anchors.fill: textField
        visible: !textField.activeFocus
        onPressed: {
            textField.forceActiveFocus();
        }
    }

    Connections {
        target: window
        function onScreenClicked(x, y) {
            if(textField.focus) window.removeFocus(textField, x, y)
        }
    }
}
