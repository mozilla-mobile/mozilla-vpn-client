/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Window 2.2
import lottie 0.1

Window {
    visible: true
    width: 640
    height: 360
    title: qsTr("Hello World")

    LottieAnimation {
        anchors.fill: parent
        source: INPUTFILE
        playing: true
    }
}
