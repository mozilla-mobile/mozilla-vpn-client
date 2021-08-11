/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.0
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14

ShaderEffect {
    id: ringAnimation

    property bool isCurrentyVisible: false
    property real animationProgress

    animationProgress: 0.0
    blending: true
    height: parent.height
    width: parent.width

    Timer {
        id: ringAnimationTimer

        interval: 20
        running: ringAnimation.visible && isCurrentyVisible
        repeat: true

        onTriggered: {
            const animationSpeed = 0.0015;
            if (ringAnimation.animationProgress < 1.0) {
                ringAnimation.animationProgress += animationSpeed
            } else {
                ringAnimation.animationProgress = 0.0;
            }
        }
    }

    fragmentShader: "
        varying mediump vec2 qt_TexCoord0;
        uniform lowp float qt_Opacity;
        uniform lowp float animationProgress;

        float draw_circle(vec2 uv, vec2 position, float radius) {
            float centerDistance = length(uv - position);
            float antialias = 0.005;

            return smoothstep(radius, radius - antialias, centerDistance);
        }

        float compose_ring(vec2 uv, vec2 position, float strokeWidth, float radius) {
            float circle1 = draw_circle(uv, position, radius);
            float circle2 = draw_circle(uv, position, radius - strokeWidth * smoothstep(0.0, 1.0, radius));

            return circle1 - circle2;
        }

        float calc_ring_radius(float minRadius, float maxRadius, float currentRadius, float offset) {
            return mod(maxRadius * offset + currentRadius, maxRadius) + minRadius;
        }

        float calc_opacity(float radius) {;
             float peak = radius / 0.1;
             return peak * exp(1.0 - peak);
        }

        void main() {
            // Center coordinate
            vec2 center = vec2(0.5, 0.275);

            // Ring properties
            float strokeWidth = 0.03;
            float minRadius = 0.0;
            float maxRadius = 1.0;
            vec4 color = vec4(1.0, 1.0, 1.0, 1.0);

            // Ring 1
            float ringRadius1 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.0);
            float ring1 = compose_ring(qt_TexCoord0, center, strokeWidth, ringRadius1) * calc_opacity(ringRadius1);
            // Ring 2
            float ringRadius2 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.33);
            float ring2 = compose_ring(qt_TexCoord0, center, strokeWidth, ringRadius2) * calc_opacity(ringRadius2);
            // Ring 3
            float ringRadius3 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.66);
            float ring3 = compose_ring(qt_TexCoord0, center, strokeWidth, ringRadius3) * calc_opacity(ringRadius3);

            // Output rings
            float rings = ring1 + ring2 + ring3;
            gl_FragColor = color * rings;
        }
    "

    RowLayout {
        anchors {
            bottom: ringAnimation.bottom
            horizontalCenter: parent.horizontalCenter
        }

        Button {
            text: ringAnimationTimer.running ? "pause" : "start"
            onClicked: {
                ringAnimationTimer.running ? ringAnimationTimer.stop() : ringAnimationTimer.start();
            }
        }

        Button {
            text: "reset"

            onClicked: {
                ringAnimation.animationProgress = 0.0;
            }
        }
    }

}

