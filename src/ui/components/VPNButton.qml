/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import themes 0.1

VPNButtonBase {
    id: button

    height: Theme.rowHeight
    Layout.preferredHeight: Layout ? Theme.rowHeight : undefined
    width: Math.min(parent.width * 0.83, Theme.maxHorizontalContentWidth)
    Layout.preferredWidth: Layout ? Math.min(parent.width * 0.83, Theme.maxHorizontalContentWidth) : undefined
    Layout.alignment: Layout ? Qt.AlignHCenter : undefined
    Component.onCompleted: {
        state = uiState.stateDefault;
    }

    VPNUIStates {
        colorScheme: Theme.blueButton
        setMargins: -5
    }

    VPNButtonLoader {
        id: loader
        state: loaderVisible ? "active" : "inactive"
    }

    VPNMouseArea {
        hoverEnabled: loaderVisible === false
    }

    contentItem: Label {
        id: label

        color: Theme.white
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        width: button.width
        font.family: Theme.fontBoldFamily
        font.pixelSize: Theme.fontSize
    }

}
