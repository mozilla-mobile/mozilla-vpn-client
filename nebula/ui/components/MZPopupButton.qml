/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

MZButtonBase {
    id: button

    property alias buttonText: buttonText.text
    property var buttonType: MZButton.ButtonType.Normal
    property var uiState:MZTheme.theme.uiState
    property bool isCancelBtn

    // Private property, will be changed depending on buttonType
    QtObject {
        id: style
        property var colorScheme: (buttonType === MZButton.ButtonType.Normal) ? MZTheme.colors.normalButton : MZTheme.colors.destructiveButton
    }

    enabled: popup.visible
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredHeight: MZTheme.theme.rowHeight
    Accessible.name: buttonText.text
    state: "state-default"

    Component.onCompleted: state = uiState.stateDefault

    MZUIStates {
        borderWidth: 1
        colorScheme: style.colorScheme
    }

    MZMouseArea {
        targetEl: button
    }

    contentItem: MZInterLabel {
        id: buttonText

        color: style.colorScheme.fontColor
        font.family: isCancelBtn ? MZTheme.theme.fontInterFamily : MZTheme.theme.fontBoldFamily
        lineHeight: 15
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}

