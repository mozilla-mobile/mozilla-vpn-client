/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

import compat 0.1

VPNAnimatedRingsShader {
    id: ringAnimation

    property bool isCurrentyVisible: false
    property bool startAnimation: false
    property real animationProgress
    property real animationOpacity

    anchors.horizontalCenter: parent.horizontalCenter
    animationProgress: 0.0
    animationOpacity: ringAnimationTimer.running ? 1.0 : 0.5
    blending: true
    height: parent.height
    width: parent.height

    Timer {
        id: ringAnimationTimer

        interval: 25
        repeat: true
        running: ringAnimation.visible && isCurrentyVisible

        onTriggered: {
            const animationSpeed = 0.001;
            if (ringAnimation.animationProgress < 1.0) {
                ringAnimation.animationProgress += animationSpeed
            } else {
                ringAnimation.animationProgress = 0.0;
            }
        }
    }

    onIsCurrentyVisibleChanged: {
        resetAnimation();
    }

    onStartAnimationChanged: {
        resetAnimation();
    }

    Behavior on animationOpacity {
        PropertyAnimation {
            duration: 1750
            easing.type: Easing.OutCirc
        }
    }

    function resetAnimation() {
        animationProgress = 0.0;
    }
}
