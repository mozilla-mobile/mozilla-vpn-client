/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

import Mozilla.VPN 1.0
import thirdparty.lottie 0.1

LottieAnimation {
    id: lottieAnimation

    anchors.fill: parent
    fillMode: Image.PreserveAspectFit
    loops: Animation.Infinite
    running: false
}
