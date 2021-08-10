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

        ShaderEffect {
            id: ringAnimation
            height: 200
            width: 200

            animatedColor: "#ff0000"

            property real time: 0.5
            property color animatedColor
            SequentialAnimation on animatedColor {
                loops: Animation.Infinite

                ColorAnimation { from: "#0000ff"; to: "#00ffff"; duration: 1000 }
                ColorAnimation { from: "#00ffff"; to: "#00ff00"; duration: 1000 }
                ColorAnimation { from: "#00ff00"; to: "#00ffff"; duration: 1000 }
                ColorAnimation { from: "#00ffff"; to: "#0000ff"; duration: 1000 }
            }

//            Timer {
//                interval: 20
//                running: true
//                repeat: true
//                onTriggered: {
//                    ringAnimation.time += 0.1
//                }
//            }

            blending: false
//            // Color example
//            fragmentShader: "
//                varying mediump vec2 qt_TexCoord0;
//                uniform lowp float qt_Opacity;
//                uniform lowp vec4 animatedColor;

//                void main() {
//                    // Set the RGBA channels of animatedColor as our fragment output
//                    gl_FragColor = animatedColor * qt_Opacity;

//                    // qt_TexCoord0 is (0, 0) at the top-left corner, (1, 1) at the
//                    // bottom-right, and interpolated for pixels in-between.
//                    if (qt_TexCoord0.x < 0.25) {
//                        // Set the green channel to 0.0, only for the left 25% of the item
//                        gl_FragColor.g = 0.0;
//                    }
//                }
//            "
//            // Working circle
//            fragmentShader: "
//                varying mediump vec2 qt_TexCoord0;
//                uniform lowp float qt_Opacity;
//                uniform lowp vec4 animatedColor;

//                uniform vec2 u_resolution;

//                float draw_circle(vec2 coord, float radius) {
//                    return step(length(coord), radius);
//                }

//                void main() {
//                    vec2 coord = gl_FragCoord.xy / u_resolution;
//                    vec2 offset = vec2(0.5, 0.5);
//                    float circle = draw_circle(qt_TexCoord0 - offset, 0.3);
//                    vec3 color = vec3(animatedColor);

////                    gl_FragColor = animatedColor * qt_Opacity;
//                    gl_FragColor = vec4(color * circle, 1.0);
//                }
//            "
            fragmentShader: "
                varying mediump vec2 qt_TexCoord0;
                uniform vec2 u_resolution;
                uniform lowp float qt_Opacity;

                uniform lowp vec4 animatedColor;
                uniform lowp float time;

                float draw_circle(vec2 coord, float radius) {
                    return step(length(coord), radius);
                }

                float circle(vec2 uv, vec2 position, float radius) {
                    float centerDistance = length(uv - position);
                    float antialias = 0.005;

                    return smoothstep(radius, radius - antialias, centerDistance);
                }

                float ring(vec2 uv, vec2 position, float strokeWidth, float radius) {
                    float circle1 = circle(uv, position, radius);
                    float circle2 = circle(uv, position, radius - strokeWidth);

                    return circle1 - circle2;
                }

                float calcRingRadius(float minRadius, float maxRadius, float speed, float delay) {
                    return mod((maxRadius - minRadius) * delay + speed, maxRadius - minRadius) + minRadius;
                }

                void main() {
                    vec2 uv = qt_TexCoord0;
                    vec2 coord = gl_FragCoord.xy / u_resolution;

                    // Center coordinate
                    vec2 center = vec2(0.5, 0.5);

                    // Ring properties
                    vec3 color = vec3(animatedColor);
                    float animationSpeed = time * 0.05;
                    float strokeWidth = 0.05;
                    float minRadius = 0.0;
                    float maxRadius = 1.05;

                    // Ring 1
                    float ringRadius1 = calcRingRadius(minRadius, maxRadius, animationSpeed, 0.0);
                    float ring1 = ring(uv, center, strokeWidth, ringRadius1);

                    // Ring 2
                    float ringRadius2 = calcRingRadius(minRadius, maxRadius, animationSpeed, 0.33);
                    float ring2 = ring(uv, center, strokeWidth, ringRadius2);

                    // Ring 3
                    float ringRadius3 =  calcRingRadius(minRadius, maxRadius, animationSpeed, 0.66);
                    float ring3 = ring(uv, center, strokeWidth, ringRadius3);

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
