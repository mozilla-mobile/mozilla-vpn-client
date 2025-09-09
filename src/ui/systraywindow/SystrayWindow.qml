/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15

import Mozilla.Shared 1.0

ApplicationWindow {
    id: root
    
    // Window properties
    width: 350
    height: 100
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    color: "transparent"
    
    // Background to catch clicks outside the card for closing
    MouseArea {
        anchors.fill: parent
        
        // Center the card in the window
        SystrayCard {
            id: card
            anchors.fill: parent
            anchors.centerIn: parent
        }
    }
    
}
