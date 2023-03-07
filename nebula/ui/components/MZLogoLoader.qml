/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5

Item {
    property bool showLoader: false

    id: logoLoader

    height: parent.height
    opacity: visible ? 1 : 0
    visible: showLoader
    width: parent.width

    onVisibleChanged: {
        if (visible) {
            loadingAnimation.play();
        } else {
            loadingAnimation.stop();
        }
    }

    MZLottieAnimation {
        id: loadingAnimation
        source: ":/nebula/resources/animations/vpnlogo-kinetic_animation.json"
    }


    Behavior on opacity {
        NumberAnimation {
            duration: 300
        }
    }
}
