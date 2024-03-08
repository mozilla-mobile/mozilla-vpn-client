/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

ColumnLayout {
    id: root
    objectName: "onboardingDataSlide"

    property string telemetryScreenId: "data_collection"

    signal nextClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideHeader2
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideBody2
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
   }

    RowLayout {
        Layout.topMargin: MZTheme.theme.vSpacingSmall
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        MZBoldInterLabel {
            id: shareDataLabel
            Layout.fillWidth: true

            text: MZI18n.OnboardingDataSlideToggleLabel
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: MZTheme.theme.labelLineHeight
        }

        MZSettingsToggle {
            objectName: "dataCollectionToggle"

            checked: MZSettings.onboardingDataCollectionEnabled
            onClicked: MZSettings.onboardingDataCollectionEnabled = !MZSettings.onboardingDataCollectionEnabled
            accessibleName: shareDataLabel.text
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    Image {
        Layout.alignment: Qt.AlignHCenter

        source: "qrc:/ui/resources/data-collection.svg"
        sourceSize: MZUiUtils.isLargePhone() ? Qt.size(209,144) : Qt.size(151,104)
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    MZButton {
        objectName: "dataNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true
        text: MZI18n.GlobalNext
        onClicked: {
            Glean.interaction.continueSelected.record({
                screen: root.telemetryScreenId,
            });

            root.nextClicked()
        }
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideLearnMoreCaption2
        color: MZTheme.theme.fontColor
   }

    MZLinkButton {
        objectName: "dataPrivacyLink"

        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZUiUtils.isMobile() ? MZTheme.theme.windowMargin * 2 : MZTheme.theme.windowMargin
        Layout.fillWidth: true

        labelText: MZI18n.InAppSupportWorkflowPrivacyNoticeLinkText
        onClicked: {
            Glean.interaction.privacyNoticeSelected.record({
                screen: root.telemetryScreenId,
            });

            MZUrlOpener.openUrlLabel("privacyNotice")
        }
    }
}
