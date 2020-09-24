import QtQuick 2.0
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.11

import Mozilla.VPN 1.0

Rectangle {
    property var yCenter: logo.y + (logo.sourceSize.height / 2) - 1
    id: animationWrapper

    anchors.fill: box
    radius: box.radius
    color: "transparent"
    visible: false
    opacity: 0

    Canvas {
        property bool startAnimation: false
        onStartAnimationChanged: animatedRings.requestPaint();

        property real maxRadius: 160
        property real startingRadius: 40
        property real startingBorderWidth: 0

        property real ring1Radius;
        property real ring1BorderWidth;

        property real ring2Radius;
        property real ring2BorderWidth;

        property real ring3Radius;
        property real ring3BorderWidth;

        property bool drawingRing2: false
        property var drawingRing3: false

        property var ringXCenter: parent.width / 2
        property var ringYCenter: animationWrapper.yCenter

        id: animatedRings
        opacity: .1
        height: animationWrapper.height
        width: animationWrapper.width
        anchors.fill: animationWrapper
        onRing1RadiusChanged: animatedRings.requestPaint()
        renderStrategy: Canvas.Threaded
        contextType: "2d"

        function updateRing(rRadius, rBorderWidth) {
            if (rRadius >= maxRadius) { // Restore to factory defaults
                rRadius = startingRadius;
                rBorderWidth = startingBorderWidth;
            }

            // Increase border width quickly on new ring creation
            if (rRadius < 115 && rBorderWidth <= 4.5) {
                rBorderWidth += 0.15;
            }

            // Start decrementing ring border width
            if (rRadius >= 115 && rBorderWidth >= -0.1) {
                rBorderWidth -= 0.05;
            }

            if (rRadius >= 135) {
                rRadius += 0.45;
            } else {
                rRadius += 0.5;
            }
            return { rRadius, rBorderWidth };
        }

        function updateRing1() {
            let {rRadius, rBorderWidth} = updateRing(ring1Radius, ring1BorderWidth);
            ring1Radius = rRadius;
            ring1BorderWidth = rBorderWidth;
        }

        function updateRing2() {
            let {rRadius, rBorderWidth} = updateRing(ring2Radius, ring2BorderWidth);
            ring2Radius = rRadius;
            ring2BorderWidth = rBorderWidth;
        }

        function updateRing3() {
            let {rRadius, rBorderWidth} = updateRing(ring3Radius, ring3BorderWidth);
            ring3Radius = rRadius;
            ring3BorderWidth = rBorderWidth;
        }

        function animateRings() {
            updateRing1();
            if (drawingRing2) {
                updateRing2();
            }
            if (drawingRing3) {
                updateRing3();
            }
        }

        function drawRing(ctx, ringRadius, borderWidth) {
            ctx.beginPath();
            ctx.arc(ringXCenter, ringYCenter, ringRadius, 0, Math.PI * 2, true)
            ctx.lineWidth =  borderWidth;
            ctx.strokeStyle = "#FFFFFF";
            ctx.closePath();
            ctx.stroke();
        }

        function resetRingValues() {
            ring1Radius = startingRadius
            ring2Radius = startingRadius
            ring3Radius = startingRadius

            ring1BorderWidth = startingBorderWidth
            ring2BorderWidth = startingBorderWidth
            ring3BorderWidth = startingBorderWidth

            drawingRing2 = false
            drawingRing3 = false
            return;
        }

        onPaint: {
            let ctx = getContext("2d")
            ctx.reset();

            if (!startAnimation) {
                resetRingValues();
                return;
            }

            // Draw first ring
            drawRing(ctx, ring1Radius, ring1BorderWidth);

            // Draw second ring when the first ring's radius is 80
            if (!drawingRing2 && ring1Radius === 80) {
                drawingRing2 = true;
            }

            // Draw third ring when the second ring's radius is 80
            if (!drawingRing3 && ring2Radius === 80) {
                drawingRing3 = true;
            }

            if (drawingRing2) {
              drawRing(ctx, ring2Radius, ring2BorderWidth);
            }

            if (drawingRing3) {
                drawRing(ctx, ring3Radius, ring3BorderWidth);
            }
            animatedRings.requestAnimationFrame(animateRings)
        }
    }

    Rectangle {
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: animationWrapper.horizontalCenter
        y: 39
        height: 100
        width: 100
        radius: 50
        color: "#321C64"
    }

    RadialGradient {
        id: bgGradient
          anchors.fill: animationWrapper
          verticalOffset: -68
          gradient: Gradient {
              GradientStop { position: 0.26; color: "transparent"}
              GradientStop { position: 0.48; color: "#321C64" }
          }
          layer.enabled: true
          layer.effect: OpacityMask {
              maskSource: Item {
                          width: animationWrapper.width
                          height: animationWrapper.height
                          Rectangle {
                              anchors.fill: parent
                              radius: animationWrapper.radius
                          }
                      }
          }
      }

    state: VPNController.state === VPNController.StateOn ? "active" : "inactive"
    states: [
        State {
            name: "active"
            PropertyChanges {
                target: animationWrapper
                visible: true
                opacity: 1;
            }
            PropertyChanges {
                target: animatedRings
                startAnimation: true
            }
        },
        State {
            name: "inactive"
            PropertyChanges {
                target: animatedRings
                startAnimation: false
            }
        }
    ]

    transitions: [
        Transition {
            to: VPNController.StateOn
            SequentialAnimation {
                PropertyAction {
                    target: animationWrapper
                    property: "visible"
                    value: true
                }
                PropertyAnimation {
                    target: animationWrapper
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                }
            }
       }
    ]
}
