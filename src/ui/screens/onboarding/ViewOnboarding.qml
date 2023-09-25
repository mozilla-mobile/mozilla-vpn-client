/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

MZStepNavigation {
    id: stepNav

    objectName: "viewOnboarding"

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        bottom: parent.bottom
        topMargin: MZTheme.theme.vSpacing
    }

    currentIndex: MZSettings.onboardingStep
    onCurrentIndexChanged: MZSettings.onboardingStep = currentIndex

    views: [
        OnboardingDataSlide {
            property string labelText: "OnboardingProgressBarDataUse"
            property string iconSource: "qrc:/nebula/resources/lock.svg"
            property string objectName: "data"

            onNextClicked: stepNav.next()
        },
        OnboardingPrivacySlide {
            property string labelText: "OnboardingProgressBarMorePrivacy"
            property string iconSource: "qrc:/ui/resources/settings/privacy.svg"
            property string objectName: "privacy"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        OnboardingDevicesSlide {
            property string labelText: "OnboardingProgressBarAddDevices"
            property string iconSource: "qrc:/ui/resources/devices.svg"
            property string objectName: "devices"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        //Last slide is different based on platform
        Loader {
            property string labelText: "OnboardingProgressBarGetStarted2"
            property string iconSource: "qrc:/nebula/resources/startup.svg"
            property string objectName: "start"

            //DEBUGGING - FIX BEFORE COMMITTING
            sourceComponent: Qt.platform.os === "android" || Qt.platform.os === "ios" ? startSlideMobile : startSlideDesktop
        }
    ]

    Component {
        id: startSlideDesktop

        OnboardingStartSlideDesktop {
            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        }
    }

    Component {
        id: startSlideMobile

        OnboardingStartSlideMobile {
            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        }
    }
}
