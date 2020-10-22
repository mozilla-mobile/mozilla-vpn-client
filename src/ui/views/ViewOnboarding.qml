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

    SwipeView {
        id: swipeView

        currentIndex: 0
        height: onboardingPanel.panelHeight
        width: parent.width

        Repeater {
            id: repeater

            model: 4

            Loader {
                height: onboardingPanel.panelHeight
                width: onboardingPanel.panelWidth
                active: SwipeView.isCurrentItem

                sourceComponent: VPNPanel {
                    height: onboardingPanel.panelHeight
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
        id: getHelp

        //% "Skip"
        labelText: qsTrId("vpn.onboarding.skip")
        onClicked: stackview.pop()
    }

    VPNButton {
        id: nextPanel

        //% "Next"
        readonly property var textNext: qsTrId("vpn.onboarding.next")

        width: 282
        text: swipeView.currentIndex === swipeView.count - 1 ? qsTrId("vpn.main.getStarted") : textNext
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progressIndicator.top
        anchors.bottomMargin: 32
        radius: 4
        onClicked: swipeView.currentIndex < swipeView.count - 1 ? swipeView.currentIndex++ : VPN.authenticate()
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
