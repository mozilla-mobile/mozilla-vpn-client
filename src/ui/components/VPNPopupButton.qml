/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import "../themes/themes.js" as Theme

VPNButtonBase {
    id: button

    property alias buttonText: buttonText.text
    property alias buttonTextColor: buttonText.color
    property var colorScheme
    property var uiState:Theme.uiState

    enabled: popup.visible
    Layout.fillWidth: true
    Layout.fillHeight: true
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

        lineHeight: 15
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap

    }

}

