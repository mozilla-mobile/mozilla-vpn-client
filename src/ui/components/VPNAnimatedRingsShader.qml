/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0

ShaderEffect {
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
        running: ringAnimation.visible && isCurrentyVisible
        repeat: true

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

    fragmentShader: "qrc:/ui/resources/shaders/baked/animatedRings.frag.qsb"

    function resetAnimation() {
        animationProgress = 0.0;
    }
}

