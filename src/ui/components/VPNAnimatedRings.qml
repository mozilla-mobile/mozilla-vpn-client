/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15
import QtQuick.Layouts 1.15
import QtQml 2.15

import Mozilla.VPN 1.0

Rectangle {
    id: animatedRingsWrapper

    property var yCenter: logo.y + 40 - 1
    property bool startAnimation: false
    property bool isCurrentyVisible: true
    property bool canRender:true

    property var animationPuffer: 0
    onIsCurrentyVisibleChanged: {
        // In case we got Visible, start a delay-timer
        // so that we switch canRender to true after 300ms.
        // - Otherwise this animation will require Draw-Cycles
        // that might be needed for the pan-navigation-animation.
        if(isCurrentyVisible){
            canRenderTimer.start();
            return;
        }
        canRender=false;
    }

    Timer {
        interval: 300
        id: canRenderTimer
        running: false
        repeat: false
        onTriggered: { canRender = true;}
    }

    onStartAnimationChanged: animatedRings.requestPaint()
    anchors.fill: box
    radius: box.radius
    color: "transparent"
    antialiasing: true
    visible: false

    Canvas {
        id: animatedRings

        property real maxRadius: 175
        property real startNextRing: 95
        property real startingRadius: 50
        property real startingBorderWidth: 1
        property real ring1Radius
        property real ring1BorderWidth
        property real ring2Radius
        property real ring2BorderWidth
        property real ring3Radius
        property real ring3BorderWidth
        property bool drawingRing2
        property var drawingRing3
        property var ringXCenter: parent.width / 2
        property var ringYCenter: animatedRingsWrapper.yCenter

        function updateRing(rRadius, rBorderWidth) {
            if (rRadius >= maxRadius) {
                // Restore to factory defaults
                rRadius = startingRadius;
                rBorderWidth = startingBorderWidth;
            }
            //Increase border width quickly on new ring creation
            if (rRadius < 115 && rBorderWidth <= 4.5)
                rBorderWidth += 0.15;

            // Start decrementing ring border width
            if (rRadius >= 115 && rBorderWidth >= 0.1)
                rBorderWidth -= 0.05;

            if (rRadius >= 135)
                rRadius += 0.45;
            else
                rRadius += 0.5;
            return {
                "rRadius": rRadius,
                "rBorderWidth": rBorderWidth
            };
        }

        function updateRing1() {
            let data = updateRing(ring1Radius, ring1BorderWidth);
            ring1Radius = data.rRadius;
            ring1BorderWidth = data.rBorderWidth;
        }

        function updateRing2() {
            let data = updateRing(ring2Radius, ring2BorderWidth);
            ring2Radius = data.rRadius;
            ring2BorderWidth = data.rBorderWidth;
        }

        function updateRing3() {
            let data = updateRing(ring3Radius, ring3BorderWidth);
            ring3Radius = data.rRadius;
            ring3BorderWidth = data.rBorderWidth;
        }

        function animateRings() {
            if(!canRender){
                return;
            }
            updateRing1();
            if (drawingRing2)
                updateRing2();

            if (drawingRing3)
                updateRing3();
        }

        Timer {
            interval: 20
            running: animatedRingsWrapper.startAnimation
            repeat: true
            onTriggered: animatedRings.animateRings()
        }

        function drawRing(ctx, ringRadius, borderWidth) {
            ctx.beginPath();
            ctx.arc(ringXCenter, ringYCenter, ringRadius, 0, Math.PI * 2, true);
            ctx.lineWidth = borderWidth;
            ctx.strokeStyle = "#FFFFFF";
            ctx.closePath();
            ctx.stroke();
        }

        function resetRingValues() {
            ring1Radius = startingRadius;
            ring2Radius = startingRadius;
            ring3Radius = startingRadius;
            ring1BorderWidth = startingBorderWidth;
            ring2BorderWidth = startingBorderWidth;
            ring3BorderWidth = startingBorderWidth;
            drawingRing2 = false;
            drawingRing3 = false;
        }

        opacity: 0.1
        height: animatedRingsWrapper.height
        width: animatedRingsWrapper.width
        anchors.fill: animatedRingsWrapper
        onRing1RadiusChanged: animatedRings.requestPaint()
        renderStrategy: Canvas.Threaded
        contextType: "2d"
        onPaint: {
            // Dont paint if not needed
            if(!canRender){
                return
            }
            let ctx = getContext("2d");
            ctx.reset();
            if (!animatedRingsWrapper.startAnimation) {
                resetRingValues();
                return ;
            }
            // Draw first ring
            drawRing(ctx, ring1Radius, ring1BorderWidth);
            // Draw second ring when the first ring's radius is 95
            if (!drawingRing2 && ring1Radius === startNextRing)
                drawingRing2 = true;

            // Draw third ring when the second ring's radius is 95
            if (!drawingRing3 && ring2Radius === startNextRing)
                drawingRing3 = true;

            if (drawingRing2)
                drawRing(ctx, ring2Radius, ring2BorderWidth);

            if (drawingRing3)
                drawRing(ctx, ring3Radius, ring3BorderWidth);
        }

        Component.onCompleted: {
            resetRingValues();
        }
    }

    Rectangle {
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: animatedRingsWrapper.horizontalCenter
        y: 45
        height: 90
        width: 90
        radius: 50
        color: "#321C64"
        antialiasing: true
    }

    RadialGradient {
        id: bgGradient

        antialiasing: true
        anchors.fill: animatedRingsWrapper
        verticalOffset: -68
        layer.enabled: true

        gradient: Gradient {
            GradientStop {
                position: 0.26
                color: "transparent"
            }

            GradientStop {
                position: 0.5
                color: "#321C64"
            }

        }

        layer.effect: OpacityMask {

            maskSource: Item {
                width: animatedRingsWrapper.width
                height: animatedRingsWrapper.height

                Rectangle {
                    anchors.fill: parent
                    radius: animatedRingsWrapper.radius
                }

            }

        }

    }

}
