/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtGraphicalEffects 1.15
import Mozilla.VPN 1.0
import QtQuick.Controls 2.15
import "../components"
import "../themes/themes.js" as Theme
import "../resources/onboarding/onboardingCopy.js" as PanelCopy

Item {
    id: onboardingPanel

    property real panelHeight: window.height - (nextPanel.height + nextPanel.anchors.bottomMargin + progressIndicator.height + progressIndicator.anchors.bottomMargin)
    property real panelWidth: window.width

    width: Math.min(parent.width, Theme.maxHorizontalContentWidth)
    anchors.horizontalCenter: parent.horizontalCenter

    SwipeView {
        id: swipeView

        currentIndex: 0
        height: onboardingPanel.panelHeight
        width: onboardingPanel.width - Theme.windowMargin * 4
        anchors.horizontalCenter: onboardingPanel.horizontalCenter

        Repeater {
            id: repeater

            model: 4
            anchors.fill: parent

            Loader {
                height: onboardingPanel.panelHeight
                active: SwipeView.isCurrentItem

                sourceComponent: VPNPanel {
                    logo: "../resources/onboarding/onboarding" + (index + 1) + ".svg"
                    logoTitle: (PanelCopy.onboardingCopy["onboarding" + (index + 1)].headline)
                    logoSubtitle: (PanelCopy.onboardingCopy["onboarding" + (index + 1)].subtitle)
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

        //% "Next"
        readonly property var textNext: qsTrId("vpn.onboarding.next")

        text: swipeView.currentIndex === swipeView.count - 1 ? qsTrId("vpn.main.getStarted") : textNext
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progressIndicator.top
        anchors.bottomMargin: 28
        radius: 4
        onClicked: swipeView.currentIndex < swipeView.count - 1 ? swipeView.currentIndex++ : VPN.authenticate()
    }

    PageIndicator {
        id: progressIndicator

        interactive: false
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.min(window.height * 0.08, 60) - 4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 8

        delegate: Rectangle {
            id: circle

            color: index === swipeView.currentIndex ? Theme.buttonColor : Theme.greyPressed
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
