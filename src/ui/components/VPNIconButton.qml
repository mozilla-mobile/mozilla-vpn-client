/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import themes 0.1

VPNButtonBase {
    id: iconButton
    property bool skipEnsureVisible: false
    property var accessibleName
    property var buttonColorScheme: Theme.iconButtonLightBackground

    height: Theme.rowHeight
    width: Theme.rowHeight

    Accessible.name: accessibleName
    Component.onCompleted: state = uiState.stateDefault
    onActiveFocusChanged: {
        if (!focus && toolTip.visible) {
            toolTip.close();
        }
    }

    VPNToolTip {
        id: toolTip
        text: accessibleName
    }

    VPNMouseArea {
        id: mouseArea
        onExited: toolTip.close()
    }

    VPNUIStates {
        itemToFocus: iconButton
        colorScheme: buttonColorScheme
    }

}
