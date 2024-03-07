/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1


Item {
    id: root

    property int totalAnimationFrames: 0
    property double currentAnimationFrame: 0
    property string selectedAnimationSource

    MZMenu {
        id: menu
        // Do not translate this string!
        title: "Animations playground"
        _menuOnBackClicked: () => getHelpStackView.pop()
    }

    Rectangle {
        id: animationContainer

        anchors.top: menu.bottom
        color: MZTheme.colors.grey50
        height: parent.width * 0.6
        width: parent.width

        MZLottieAnimation {
            id: lottieAnimationExample
            source: selectedAnimationSource

            Connections {
                target: lottieAnimationExample.status

                function onChanged(playing, currentTime, totalTime) {
                    totalAnimationFrames = totalTime;
                    currentAnimationFrame = currentTime;
                }
            }
        }

        MZInterLabel {
            // Do not translate this string!
            text: "No animation selected"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: MZTheme.colors.white
            visible: animationSelect.currentIndex < 0
        }

        ProgressBar {
            id: animationProgress

            anchors.top: animationContainer.bottom
            contentItem: Item {
                implicitHeight: parent.height
                implicitWidth: parent.width

                Rectangle {
                    color: MZTheme.colors.informational.default
                    height: parent.height
                    width: animationProgress.visualPosition * parent.width
                }
            }
            value: totalAnimationFrames ? currentAnimationFrame / totalAnimationFrames : 0
            width: parent.width
        }
    }

    ColumnLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: animationContainer.bottom
        anchors.topMargin: MZTheme.theme.listSpacing * 3
        spacing: MZTheme.theme.listSpacing * 2
        width: parent.width - MZTheme.theme.windowMargin * 2

        MZComboBox {
            id: animationSelect

            currentIndex: -1
            // Do not translate this string!
            placeholderText: "Select an animation"
            model: ListModel {
                id: animationItems

                ListElement {
                    name: "VPN Logo (drop)"
                    value: ":/ui/resources/animations/vpnlogo-drop_animation.json"
                }
                ListElement {
                    name: "Lock"
                    value: ":/ui/resources/animations/lock_animation.json"
                }
                ListElement {
                    name: "Globe"
                    value: ":/ui/resources/animations/globe_animation.json"
                }
                ListElement {
                    name: "VPN Active"
                    value: ":/ui/resources/animations/vpnactive_animation.json"
                }
            }

            function setCurrentAnimationSource() {
                if (currentIndex >= 0) {
                    root.selectedAnimationSource = animationItems.get(currentIndex).value;
                }
            }

            onCurrentIndexChanged: () => {
                setCurrentAnimationSource()
            }

            Component.onCompleted: {
                setCurrentAnimationSource()
            }

            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            MZButton {
                // Do not translate this string!
                text: lottieAnimationExample.status.playing ? "pause" : "play"
                enabled: animationSelect.currentIndex >= 0
                opacity: enabled ? 1 : 0.5
                onClicked: () => {
                    if (lottieAnimationExample.status.playing) {
                        lottieAnimationExample.pause();
                    } else {
                        lottieAnimationExample.play();
                    }
                }

                Layout.fillWidth: true
            }

            MZButton {
                // Do not translate this string!
                text: "reset"
                enabled: animationSelect.currentIndex >= 0 && lottieAnimationExample.status.playing
                opacity: enabled ? 1 : 0.5
                onClicked: () => {
                    lottieAnimationExample.stop();
                }

                Layout.fillWidth: true
            }
        }

    }

}
