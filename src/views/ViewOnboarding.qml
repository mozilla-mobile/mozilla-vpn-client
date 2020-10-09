/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtGraphicalEffects 1.15

import Mozilla.VPN 1.0

import "../components"
import "../themes/themes.js" as Theme
import "../resources/onboarding/onboardingCopy.js" as PanelCopy

Item {
    property var panelNum: 1
    id: onboardingPanel
    state: `onboarding${panelNum}`

    VPNHeaderLink {
        id: getHelp
        labelText: qsTr("Skip")
        onClicked: stackview.pop()
    }

    VPNPanel {
        id: contentWrapper
        logo: `../resources/onboarding/${onboardingPanel.state}.svg`
        logoTitle:  qsTr(PanelCopy.onboardingCopy[onboardingPanel.state].headline)
        logoSubtitle: qsTr(PanelCopy.onboardingCopy[onboardingPanel.state].subtitle)

        Component.onCompleted: fade.start()
        PropertyAnimation on opacity {
            id: fade
            from: 0
            to: 1
            duration: 800
        }
    }

    VPNButton {
        property var onboardingOver: (panelNum === 4)
        id: nextPanel
        width: 282
        text: onboardingOver ? qsTr("Get started") : qsTr("Next")
        anchors.horizontalCenterOffset: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: progressIndicator.top
        anchors.bottomMargin: 32
        radius: 4
        onClicked: onboardingOver ? VPN.authenticate() : goToNextPanel()
    }

    Row {
        id: progressIndicator
        spacing: 8
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 40
        Repeater {
            model: 4
            Rectangle {
                id: circle
                color: onboardingPanel.panelNum === (index + 1) ? Theme.buttonColor : Theme.greyPressed
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

    function goToNextPanel() {
        onboardingPanel.panelNum++;
        fade.start();
    }
}
