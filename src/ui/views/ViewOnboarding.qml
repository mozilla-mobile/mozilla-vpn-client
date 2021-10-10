/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import Mozilla.VPN 1.0
import QtQuick.Controls 2.14
import "../components"
import "../themes/themes.js" as Theme

Item {
    id: onboardingPanel

    property real panelHeight: window.height - (nextPanel.height + nextPanel.anchors.bottomMargin + progressIndicator.height + progressIndicator.anchors.bottomMargin)
    property real panelWidth: window.width

    width: window.width
    anchors.horizontalCenter: window.horizontalCenter

    SwipeView {
        id: swipeView

        currentIndex: 0
        height: onboardingPanel.panelHeight
        width: onboardingPanel.width
        anchors.horizontalCenter: onboardingPanel.horizontalCenter

        ListModel {
            id: onboardingModel

            ListElement {
                image: "../resources/onboarding/onboarding1.svg"
                //% "Device-level encryption"
                headline: qsTrId("vpn.onboarding.headline.1")
                //% "Encrypt your traffic so that it can’t be read by your ISP or eavesdroppers."
                subtitle: qsTrId("vpn.onboarding.subtitle.1")
            }

            ListElement {
                image: "../resources/onboarding/onboarding2.svg"
                //: The + after the number stands for “more than”. If you change the number of countries here, please update ViewSubscriptionNeeded.qml too.
                //% "Servers in 30+ countries"
                headline: qsTrId("vpn.onboarding.headline.2")
                //% "Pick a server in any country you want and hide your location to throw off trackers."
                subtitle: qsTrId("vpn.onboarding.subtitle.2")
            }

            ListElement {
                image: "../resources/onboarding/onboarding3.svg"
                //% "No bandwidth restrictions"
                headline: qsTrId("vpn.onboarding.headline.3")
                //% "Stream, download, and game without limits, monthly caps or ISP throttling."
                subtitle: qsTrId("vpn.onboarding.subtitle.3")
            }

            ListElement {
                image: "../resources/onboarding/onboarding4.svg"
                //% "No online activity logs"
                headline: qsTrId("vpn.onboarding.headline.4")
                //% "We are committed to not monitoring or logging your browsing or network history."
                subtitle: qsTrId("vpn.onboarding.subtitle.4")
            }
        }

        Repeater {
            id: repeater

            model: onboardingModel
            anchors.fill: parent

            Loader {
                height: onboardingPanel.panelHeight
                active: SwipeView.isCurrentItem

                sourceComponent: VPNPanel {
                    logo: image
                    logoTitle: headline
                    logoSubtitle: subtitle
                    height: nextPanel.y
                    Component.onCompleted: fade.start()

                    PropertyAnimation on opacity {
                        id: fade

                        from: 0
                        to: 1
                        duration: 800
                    }

                }

            }

        }

    }

    VPNHeaderLink {
        objectName: "skipOnboarding"

        //% "Skip"
        labelText: qsTrId("vpn.onboarding.skip")
        onClicked: stackview.pop()
        visible: swipeView.currentIndex !== swipeView.count - 1
    }

    VPNButton {
        id: nextPanel
        objectName: "onboardingNext"

        //% "Next"
        readonly property var textNext: qsTrId("vpn.onboarding.next")

        text: swipeView.currentIndex === swipeView.count - 1 ? qsTrId("vpn.main.getStarted") : textNext
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: onboardingPanel.horizontalCenter
        anchors.bottom: progressIndicator.top
        anchors.bottomMargin: 28
        radius: Theme.cornerRadius
        onClicked: swipeView.currentIndex < swipeView.count - 1 ? swipeView.currentIndex++ : VPN.getStarted()
    }

    PageIndicator {
        id: progressIndicator

        interactive: false
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.min(window.height * 0.08, 60) - 4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: Theme.windowMargin / 2

        delegate: Rectangle {
            id: circle

            color: index === swipeView.currentIndex ? Theme.blue : Theme.greyPressed
            height: 6
            width: 6
            radius: 6

            Behavior on color {
                ColorAnimation {
                    duration: 400
                }

            }

        }

    }

    Component.onCompleted: VPNCloseEventHandler.addView(onboardingPanel)

    Connections {
        target: VPNCloseEventHandler
        function onGoBack(item) {
            if (item !== onboardingPanel) {
                return;
            }

            if (swipeView.currentIndex === 0) {
                stackview.pop();
                return;
            }

            swipeView.currentIndex--;
        }
    }
}
