/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1
import thirdparty.lottie 0.1


Item {
    id: root

    property string selectedAnimationSource

    anchors.fill: parent

    VPNMenu {
        id: menu
        // Do not translate this string!
        title: "Animations playground"
        isSettingsView: false
    }

    Rectangle {
        id: animationContainer

        anchors.top: menu.bottom
        color: VPNTheme.colors.grey40
        height: parent.width * 0.6
        width: parent.width

        VPNLottieAnimation {
            id: lottieAnimationExample
            source: selectedAnimationSource
        }
    }

    VPNComboBox {
        currentIndex: 0
        model: ListModel {
            id: animationItems

            ListElement {
                text: "Lock"
                name: "Lock"
                value: "qrc:/nebula/resources/animations/lock_animation.json"
                source: "qrc:/nebula/resources/animations/lock_animation.json"
            }
            ListElement {
                text: "Globe"
                name: "Globe"
                value: "qrc:/nebula/resources/animations/globe_animation.json"
                source: "qrc:/nebula/resources/animations/globe_animation.json"
            }
            ListElement {
                text: "VPN Active"
                name: "VPN Active"
                value: "qrc:/nebula/resources/animations/vpnactive_animation.json"
                source: "qrc:/nebula/resources/animations/vpnactive_animation.json"
            }
            ListElement {
                text: "Speedometer"
                name: "Speedometer"
                value: "qrc:/nebula/resources/animations/speedometer_animation.json"
                source: "qrc:/nebula/resources/animations/speedometer_animation.json"
            }
        }

        function setCurrentAnimationSource() {
            root.selectedAnimationSource = animationItems.get(currentIndex).source;
        }

        onCurrentIndexChanged: () => {
            setCurrentAnimationSource()
        }

        Component.onCompleted: {
            setCurrentAnimationSource()
        }

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: animationContainer.bottom
        anchors.topMargin: VPNTheme.theme.windowMargin
        width: parent.width - VPNTheme.theme.windowMargin * 2
    }

}
