/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1


Item {
    id: root

    property int totalAnimationFrames: ({})
    property int currentAnimationFrame: 0
    property string selectedAnimationSource

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Animations playground"
        isSettingsView: false
    }

    Rectangle {
        id: animationContainer

        anchors.top: menu.bottom
        color: VPNTheme.colors.grey50
        height: parent.width * 0.6
        width: parent.width

        VPNLottieAnimation {
            id: lottieAnimationExample
            source: selectedAnimationSource
            onEnteredFrame: (currentTime) => {
                currentAnimationFrame = currentTime;
            }
            onStatusChanged: () => {
                if (status === Image.Ready) {
                    totalAnimationFrames = lottieAnimationExample.getDuration(true);
                }
            }
        }

        VPNInterLabel {
            // Do not translate this string!
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: VPNTheme.colors.white
            text: "No animation selected"
            visible: animationSelect.currentIndex < 0
        }
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: animationContainer.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin * 2

        ProgressBar {
            id: animationProgress

            value: totalAnimationFrames ? currentAnimationFrame / totalAnimationFrames : 0
            Layout.fillWidth: true
        }

        VPNComboBox {
            id: animationSelect

            currentIndex: -1
            // Do not translate this string!
            placeholderText: "Select an animation"
            model: ListModel {
                id: animationItems

                ListElement {
                    name: "Lock"
                    value: "qrc:/nebula/resources/animations/lock_animation.json"
                }
                ListElement {
                    name: "Globe"
                    value: "qrc:/nebula/resources/animations/globe_animation.json"
                }
                ListElement {
                    name: "VPN Active"
                    value: "qrc:/nebula/resources/animations/vpnactive_animation.json"
                }
                ListElement {
                    name: "Speedometer"
                    value: "qrc:/nebula/resources/animations/speedometer_animation.json"
                }
            }

            function setCurrentAnimationSource() {
                root.selectedAnimationSource = animationItems.get(currentIndex).value;
            }

            onCurrentIndexChanged: () => {
                setCurrentAnimationSource()
            }

            Component.onCompleted: {
                setCurrentAnimationSource()
            }

            Layout.fillWidth: true
        }

        VPNButton {
            // Do not translate this string!
            text: lottieAnimationExample.running ? "pause" : "play"
            enabled: animationSelect.currentIndex >= 0
            opacity: enabled ? 1 : 0.5
            onClicked: () => {
                if (lottieAnimationExample.running) {
                    lottieAnimationExample.pause();
                } else {
                    lottieAnimationExample.start();
                }
            }

            Layout.fillWidth: true
        }

        VPNButton {
            // Do not translate this string!
            text: "reset"
            enabled: animationSelect.currentIndex >= 0 && lottieAnimationExample.running
            opacity: enabled ? 1 : 0.5
            onClicked: () => {
                lottieAnimationExample.stop();
            }

            Layout.fillWidth: true
        }

    }

}
