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

            onNextClicked: stepNav.next()
        },
        OnboardingPrivacySlide {
            property string labelText: "OnboardingProgressBarMorePrivacy"
            property string iconSource: "qrc:/ui/resources/settings/privacy.svg"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        OnboardingDevicesSlide {
            property string labelText: "OnboardingProgressBarAddDevices"
            property string iconSource: "qrc:/ui/resources/devices.svg"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        OnboardingStartSlide {
            property string labelText: "OnboardingProgressBarGetStarted"
            property string iconSource: "qrc:/nebula/resources/startup.svg"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        }
    ]
}
