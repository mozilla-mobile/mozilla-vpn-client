/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.Shared 1.0
import components 0.1

QtObject {
    id: statusBarModifier

    property int statusBarTextColor: MZTheme.StatusBarTextColorDark
    onStatusBarTextColorChanged: applyChanges();

    function applyChanges() {
        MZTheme.setStatusBarTextColor(statusBarModifier.statusBarTextColor);
    }

    function resetDefaults() {
        statusBarTextColor = MZTheme.StatusBarTextColorDark;
    }

    Component.onCompleted: applyChanges()
    Component.onDestruction: resetDefaults()
}
