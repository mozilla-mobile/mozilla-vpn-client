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

    property var panelNum: 1

    VPNHeaderLink {
        id: getHelp

        //% "Skip"
//        labelText: qsTrId("vpn.onboarding.skip")
        onClicked: stackview.pop()
    }

    SwipeView {
        id: swipeView

        property real numPanels: 4

        currentIndex: 0
        anchors.fill: parent

        Repeater {
            model: swipeView.numPanels

            Loader {
                active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem

                sourceComponent: VPNPanel {
                    id: contentWrapper

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

    VPNButton {
        id: nextPanel

        //% "Next"
        readonly property var textNext: qsTrId("vpn.onboarding.next")

        width: 282
        text: swipeView.currentIndex === 3 ? qsTrId("vpn.main.getStarted") : textNext
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progressIndicator.top
        anchors.bottomMargin: 32
        radius: 4
        onClicked: swipeView.currentIndex < 3 ? swipeView.currentIndex++ : VPN.authenticate()
    }

    PageIndicator {
        id: progressIndicator

        interactive: false
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
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

}
