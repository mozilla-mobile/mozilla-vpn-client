/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import themes 0.1

Rectangle {
    property variant focusedComponent
    property var focusColorScheme: Theme.blueButton
    property var setMargins: -3

    color: focusColorScheme.focusOutline
    antialiasing: true
    anchors.fill: parent
    anchors.margins: setMargins
    radius: parent.radius + ((setMargins  * -1) - 1)
    opacity: focusedComponent.activeFocus ? 1: 0
    z: -1
}
