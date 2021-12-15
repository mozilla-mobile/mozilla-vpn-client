/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14

import Qt.labs.lottieqt 1.0

Rectangle {
    id: root

    anchors.centerIn: parent
    color: "pink"
    height: parent.height * 0.5
    width: parent.width * 0.5


    LottieAnimation {
        id: sparkle
        // anchors.fill: parent
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        scale: 1
        source: "qrc:/nebula/resources/animations/test_animation.json"
        // source: "qrc:/nebula/resources/animations/star_animation.json"
        // source: "qrc:/nebula/resources/animations/clock_animation.json"
        // source: "qrc:/nebula/resources/animations/vpnactive_animation.json"
        autoPlay: true
        frameRate: 20
        loops: LottieAnimation.Infinite
        opacity: 1
        z: 1

        onStatusChanged: {
            console.log("onStatusChanged");

            if (status === LottieAnimation.Null) {
                console.log("LottieAnimation.Null");
            }
            if (status === LottieAnimation.Loading) {
                console.log("LottieAnimation.Loading");
            }
            if (status === LottieAnimation.Ready) {
                console.log("LottieAnimation.Ready");
            }
            if (status === LottieAnimation.Error) {
                console.log("LottieAnimation.Error");
            }
        }
    }
}
