/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0

MZButtonBase {
    enum ButtonType {
        Normal,
        Destructive
    }

    property var buttonType: MZButton.ButtonType.Normal

    property int fontSize: MZTheme.theme.fontSize
    property alias label: label
    property int wrapMode: Text.NoWrap

    id: button

    // Private property, will be changed depending on buttonType
    QtObject {
        id: style
        property var colorScheme: (buttonType === MZButton.ButtonType.Normal) ? MZTheme.colors.normalButton : MZTheme.colors.destructiveButton
    }
    height: Math.max(MZTheme.theme.rowHeight, label.implicitHeight + MZTheme.theme.rowHeight / 2)
    width: Math.min(parent.width * 0.83, MZTheme.theme.maxHorizontalContentWidth)

    Layout.alignment: Layout ? Qt.AlignHCenter : undefined
    Layout.preferredHeight: Layout ? Math.max(MZTheme.theme.rowHeight, label.implicitHeight + MZTheme.theme.rowHeight / 2) : undefined

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
        colorScheme: style.colorScheme
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

        color: style.colorScheme.fontColor
        text: button.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: button.wrapMode
        elide: button.wrapMode === Text.NoWrap ? Text.ElideRight : Text.ElideNone
        width: button.width
        font.family: MZTheme.theme.fontBoldFamily
        font.pixelSize: fontSize
        opacity: loaderVisible ? 0 : 1
        Accessible.ignored: !visible
    }

}
