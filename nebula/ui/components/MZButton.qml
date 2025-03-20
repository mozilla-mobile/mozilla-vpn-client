/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

MZButtonBase {
    property var colorScheme: MZTheme.colors.normalButton // this should either be normalButton or destructiveButton
    property int fontSize: MZTheme.theme.fontSize
    property alias label: label
    property alias buttonTextColor: label.color

    id: button

    height: MZTheme.theme.rowHeight
    width: Math.min(parent.width * 0.83, MZTheme.theme.maxHorizontalContentWidth)

    Layout.alignment: Layout ? Qt.AlignHCenter : undefined
    Layout.preferredHeight: Layout ? MZTheme.theme.rowHeight : undefined
    Layout.preferredWidth: Layout
        ? Math.min(parent.width * 0.83, MZTheme.theme.maxHorizontalContentWidth)
        : undefined

    Component.onCompleted: {
        state = Qt.binding(() => (
            enabled ? uiState.stateDefault : uiState.stateDisabled
        ));
        buttonMouseArea.hoverEnabled = Qt.binding(() => (
            enabled && loaderVisible === false
        ));
    }

    MZUIStates {
        colorScheme: button.colorScheme
        setMargins: -5
    }

    MZButtonLoader {
        id: loader
        state: loaderVisible ? "active" : "inactive"
    }

    MZMouseArea {
        id: buttonMouseArea
    }

    contentItem: Label {
        id: label

        color: colorScheme.fontColor
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        width: button.width
        font.family: MZTheme.theme.fontBoldFamily
        font.pixelSize: fontSize
        opacity: loaderVisible ? 0 : 1
        Accessible.ignored: !visible
    }

}
