/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/ui/screens/settings/privacy"
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

ColumnLayout {
    id: root
    objectName: "onboardingPrivacySlide"

    property string telemetryScreenId: "get_more_privacy"

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        Layout.fillWidth: true

        text: MZI18n.OnboardingPrivacySlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        text: MZI18n.OnboardingPrivacySlideBody
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
    }

    PrivacyFeaturesList {
        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        isOnboarding: true
        dividerSpacing: 8
        telemetryScreenId: root.telemetryScreenId
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    MZButton {
        objectName: "privacyNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.GlobalNext

        onClicked: {
            Glean.interaction.continueSelected.record({
                screen: root.telemetryScreenId,
            });

            root.nextClicked()
        }
    }

    MZLinkButton {
        objectName: "privacyBackButton"

        Layout.topMargin: 16
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZUiUtils.isMobile() ? MZTheme.theme.windowMargin * 2 : MZTheme.theme.windowMargin
        Layout.fillWidth: true

        implicitHeight: MZTheme.theme.rowHeight

        labelText: MZI18n.GlobalGoBack

        onClicked: {
            Glean.interaction.goBackSelected.record({
                screen: root.telemetryScreenId,
            });

            root.backClicked()
        }
    }
}
