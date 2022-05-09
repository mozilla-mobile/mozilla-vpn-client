/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick

Window {
    width: 100
    height: 100
    visible: true
    title: "Mozilla VPN - Checking the graphical card"

    Rectangle {
        anchors.fill: parent
        anchors.margins: 4
        color: "blue"
    }

    Component.onCompleted: () => Qt.quit()
}
