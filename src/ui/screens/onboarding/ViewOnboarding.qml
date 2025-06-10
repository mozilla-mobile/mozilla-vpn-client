/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils
import "qrc:/nebula/utils/MZAssetLookup.js" as MZAssetLookup

MZStepNavigation {
    id: stepNav
    objectName: "viewOnboarding"

    //Must be kept in sync with 'views' order (order of progress bar)
    enum Slide {
        Data,
        Privacy,
        Devices,
        Start
    }

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
        bottom: parent.bottom
        topMargin: MZUiUtils.isMobile() ? 36 : MZTheme.theme.vSpacing
    }

    currentIndex: MZSettings.onboardingStep
    onCurrentIndexChanged: MZSettings.onboardingStep = currentIndex

    views: [
        OnboardingDataSlide {
            id: dataSlide

            property string labelText: "OnboardingProgressBarDataUse"
            property string iconSource: MZAssetLookup.getImageSource("IconLock")
            property string objectName: "data"

            onNextClicked: stepNav.next()
        },
        OnboardingPrivacySlide {
            id: privacySlide

            property string labelText: "OnboardingProgressBarMorePrivacy"
            property string iconSource: MZAssetLookup.getImageSource("IconPrivacyMask")
            property string objectName: "privacy"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        OnboardingDevicesSlide {
            id: devicesSlide

            property string labelText: "OnboardingProgressBarAddDevices"
            property string iconSource: MZAssetLookup.getImageSource("IconDevices")
            property string objectName: "devices"

            onNextClicked: stepNav.next()
            onBackClicked: stepNav.back()
        },
        //Last slide is different based on platform
        Loader {
            id: startSlide

            property string labelText: MZUiUtils.isMobile() ? "OnboardingProgressBarConnect" : "OnboardingProgressBarStartup"
            property string iconSource: MZAssetLookup.getImageSource("IconPowerButton")
            property string objectName: "start"

            sourceComponent: MZUiUtils.isMobile() ? startSlideMobile : startSlideDesktop
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
