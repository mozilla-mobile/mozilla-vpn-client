/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0

VPNButtonBase {
    id: button

    height: VPNTheme.theme.rowHeight
    Layout.preferredHeight: Layout ? VPNTheme.theme.rowHeight : undefined
    width: parent.width
    Layout.preferredWidth: Layout ? Math.min(parent.width * 0.83, VPNTheme.theme.maxHorizontalContentWidth) : undefined
    Layout.alignment: Layout ? Qt.AlignHCenter : undefined
    Component.onCompleted: {
        state = uiState.stateDefault;
    }

    VPNUIStates {
        colorScheme: VPNTheme.theme.wasmOptionBtn
        setMargins: -5
    }

    VPNMouseArea {
        hoverEnabled: loaderVisible === false
    }

    contentItem: Label {
        id: label

        color: VPNTheme.theme.fontColorDark
        text: button.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        width: button.width
        font.family: VPNTheme.theme.fontFamily
        font.pixelSize: 13
    }

}
