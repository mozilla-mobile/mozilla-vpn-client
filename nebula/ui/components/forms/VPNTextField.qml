/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1

TextField {
    property bool hasError: false
    property bool showInteractionStates: true
    property bool forceBlurOnOutsidePress: true
    property alias _placeholderText: centeredPlaceholderText.text
    property string _accessibleName: _placeholderText
    property string _accessibleDescription: ""

    id: textField

    Accessible.name: _accessibleName
    Accessible.description: _accessibleDescription
    Accessible.focused: textField.focus
    Layout.alignment: Qt.AlignVCenter
    Layout.preferredHeight: VPNTheme.theme.rowHeight

    background: VPNInputBackground {
        id: textFieldBackground
    }
    bottomPadding: VPNTheme.theme.windowMargin / 2
    color: VPNTheme.colors.input.default.text
    cursorDelegate: VPNCursorDelegate {}
    echoMode: TextInput.Normal
    focus: true
    font.family: VPNTheme.theme.fontInterFamily
    font.pixelSize: VPNTheme.theme.fontSizeSmall
    inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
    leftPadding: VPNTheme.theme.windowMargin
    placeholderTextColor: VPNTheme.colors.grey40
    rightPadding: VPNTheme.theme.windowMargin
    selectByMouse: true
    topPadding: VPNTheme.theme.windowMargin / 2
    verticalAlignment: TextInput.AlignVCenter

    onActiveFocusChanged: if (focus && typeof(vpnFlickable) !== "undefined" && vpnFlickable.ensureVisible) {
        vpnFlickable.ensureVisible(textField);
    }
    // This is a workaround for VoiceOver on macOS: https://bugreports.qt.io/browse/QTBUG-108189
    // After gaining initial focus or typing in TextField the screen reader
    // fails to narrate any accessible content and action. After regaining
    // active focus the screen reader keeps working as expected.
    onTextChanged: {
        if (Qt.platform.os === "osx") {
            textField.focus = false;
            textField.forceActiveFocus();
        }
    }

    Text {
        id: centeredPlaceholderText

        Accessible.ignored: true

        color: textField.placeholderTextColor
        elide: Text.ElideRight
        font: textField.font
        height: VPNTheme.theme.rowHeight
        verticalAlignment: Text.AlignVCenter
        visible: !textField.length && !textField.preeditText
        width: textField.width - (textField.leftPadding + textField.rightPadding)
        x: textField.leftPadding
    }

    VPNInputStates {
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
}
