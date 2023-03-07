/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1


MZViewBase {
    _menuTitle: "Theme list"

    _viewContentData: ColumnLayout {
        id: themeListHolder

        Layout.fillWidth: true
        Layout.margins: MZTheme.theme.windowMargin
        spacing: MZTheme.theme.windowMargin

        ButtonGroup {
            id: radioButtonGroup
        }

        Repeater {
            Layout.fillWidth: true
            model: MZTheme

            delegate: MZRadioDelegate {
                radioButtonLabelText: name
                accessibleName: name
                checked: MZTheme.currentTheme === name
                onClicked: MZTheme.currentTheme = name
            }
        }
    }
}
