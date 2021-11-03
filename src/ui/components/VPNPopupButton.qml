/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import themes 0.1

VPNButtonBase {
    id: button

    property alias buttonText: buttonText.text
    property alias buttonTextColor: buttonText.color
    property var colorScheme
    property var uiState:Theme.uiState
    property var isCancelBtn

    enabled: popup.visible
    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.preferredHeight: Theme.rowHeight
    Accessible.name: buttonText.text
    state: "state-default"

    Component.onCompleted: state = uiState.stateDefault

    VPNUIStates {
        borderWidth: 1
        colorScheme: parent.colorScheme
    }

    VPNMouseArea {
        targetEl: button
    }

    contentItem: VPNInterLabel {
        id: buttonText

        font.family: isCancelBtn ? Theme.fontInterFamily : Theme.fontBoldFamily
        lineHeight: 15
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}

