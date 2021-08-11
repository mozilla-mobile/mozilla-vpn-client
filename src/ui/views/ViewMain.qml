/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import QtQuick.Layouts 1.14
import Mozilla.VPN 1.0
import "../components"
import "../themes/themes.js" as Theme

VPNFlickable {
    id: vpnFlickable

    flickContentHeight: col.height + Theme.windowMargin / 2
    states: [
        State {
            when: window.fullscreenRequired()

            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: true
            }
        },
        State {
            when: !window.fullscreenRequired()
            PropertyChanges {
                target: mainView
            }
            PropertyChanges {
                target: mobileHeader
                visible: false
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        enabled: box.connectionInfoVisible
        onClicked: box.closeConnectionInfo()
    }


    GridLayout {
        id: col
        width: parent.width - Theme.windowMargin
        anchors.horizontalCenter: parent.horizontalCenter
        flow: GridLayout.TopToBottom
        rowSpacing: Theme.windowMargin
        anchors.top: parent.top
        anchors.topMargin: Theme.windowMargin

        RowLayout {
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

        ShaderEffect {
            property real animationProgress
            property color animatedColor
            property color ringColor

            id: ringAnimation
            height: parent.width
            width: parent.width

            animationProgress: 0.0
            ringColor: "#ffffff"

            Timer {
                id: ringAnimationTimer

                interval: 20
                running: true
                repeat: true
                onTriggered: {
                    const animationSpeed = 0.0015;
                    if (ringAnimation.animationProgress < 1.0) {
                        ringAnimation.animationProgress += animationSpeed;
                    } else {
                        ringAnimation.animationProgress = 0.0;
                    }
                }
            }

            blending: false
            fragmentShader: "
                varying mediump vec2 qt_TexCoord0;
                uniform lowp float qt_Opacity;

                uniform lowp vec4 animatedColor;
                uniform lowp vec4 ringColor;
                uniform lowp float animationProgress;

                float draw_circle(vec2 uv, vec2 position, float radius) {
                    float centerDistance = length(uv - position);
                    float antialias = 0.005;

                    return smoothstep(radius, radius - antialias, centerDistance);
                }

                float compose_ring(vec2 uv, vec2 position, float strokeWidth, float radius) {
                    float circle1 = draw_circle(uv, position, radius);
                    float circle2 = draw_circle(uv, position, radius - strokeWidth * smoothstep(0.0, 1.0, radius * 5.0));

                    return circle1 - circle2;
                }

                float calc_ring_radius(float minRadius, float maxRadius, float currentRadius, float offset) {
                     return mod(maxRadius * offset + currentRadius, maxRadius) + minRadius;
                }

                float calc_opacity(float radius) {;
                    float peak = radius / 0.2;
                    return peak * exp(0.25 - peak);
                }

                void main() {
                    vec2 uv = qt_TexCoord0;

                    // Center coordinate
                    vec2 center = vec2(0.5, 0.5);

                    // Ring properties
                    vec3 color = vec3(ringColor * qt_Opacity);
                    float strokeWidth = 0.015;
                    float minRadius = 0.0;
                    float maxRadius = 1.0;

                    // Ring 1
                    float ringRadius1 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.0);
                    float ring1 = compose_ring(uv, center, strokeWidth, ringRadius1) * calc_opacity(ringRadius1);
                    // Ring 2
                    float ringRadius2 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.33);
                    float ring2 = compose_ring(uv, center, strokeWidth, ringRadius2) * calc_opacity(ringRadius2);

                    // Ring 3
                    float ringRadius3 = calc_ring_radius(minRadius, maxRadius, animationProgress, 0.66);
                    float ring3 = compose_ring(uv, center, strokeWidth, ringRadius3) * calc_opacity(ringRadius3);

                    // Output rings
                    float rings = ring1 + ring2 + ring3;

                    gl_FragColor = vec4(color * rings, 1.0);
                }
            "
        }


        RowLayout {
            id: mobileHeader
            Layout.preferredHeight: Theme.rowHeight
            Layout.alignment: Qt.AlignHCenter
            spacing: 6

            VPNIcon {
                source: VPNStatusIcon.iconUrl
                sourceSize.height: 20
                sourceSize.width: 20
                antialiasing: true
                Layout.alignment: Qt.AlignVCenter
            }

            VPNBoldLabel {
                //% "Mozilla VPN"
                text: qsTrId("MozillaVPN")
                color: "#000000"
                Layout.alignment: Qt.AlignVCenter
            }
        }

        VPNAlerts {
            id: notifications
        }

        VPNControllerView {
            id: box

        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewServers.qml")
            }

            Layout.topMargin: 12

            id: serverInfo
            objectName: "serverListButton"

            //% "Select location"
            //: Select the Location of the VPN server
            titleText: qsTrId("vpn.servers.selectLocation")
            //% "current location - %1"
            //: Accessibility description for current location of the VPN server
            descriptionText: qsTrId("vpn.servers.currentLocation").arg(VPNCurrentServer.localizedCityName)

            subtitleText: VPNCurrentServer.localizedCityName
            imgSource:  "../resources/flags/" + VPNCurrentServer.countryCode.toUpperCase() + ".png"
            disableRowWhen: (VPNController.state !== VPNController.StateOn && VPNController.state !== VPNController.StateOff) || box.connectionInfoVisible
        }

        VPNControllerNav {
            function handleClick() {
                stackview.push("ViewDevices.qml")
            }

            Layout.topMargin: 6

            objectName: "deviceListButton"
            //% "%1 of %2"
            //: Example: You have "x of y" devices in your account, where y is the limit of allowed devices.
            subtitleText: qsTrId("vpn.devices.activeVsMaxDeviceCount").arg(VPNDeviceModel.activeDevices + (VPN.state !== VPN.StateDeviceLimit ? 0 : 1)).arg(VPNUser.maxDevices)
            imgSource: "../resources/devices.svg"
            imgIsVector: true
            imgSize: 24
            //% "My devices"
            titleText: qsTrId("vpn.devices.myDevices")
            disableRowWhen: box.connectionInfoVisible
        }

        VPNVerticalSpacer {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }
    }
}
