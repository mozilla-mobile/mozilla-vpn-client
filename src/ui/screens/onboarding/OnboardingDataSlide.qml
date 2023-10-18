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

    signal nextClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideBody
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
   }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 24
    }

    Image {
        Layout.alignment: Qt.AlignHCenter

        source: "qrc:/ui/resources/data-collection.svg"
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 24
    }

    MZCheckBoxRow {
        objectName: "dataCollectionCheckBox"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        leftMargin: 0
        subLabelText: MZI18n.OnboardingDataSlideCheckboxLabel
        showDivider: false
        isChecked: MZSettings.onboardingDataCollectionEnabled

        onClicked: MZSettings.onboardingDataCollectionEnabled = !MZSettings.onboardingDataCollectionEnabled
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 24
    }

    MZButton {
        objectName: "dataNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true
        text: MZI18n.GlobalNext
        onClicked: root.nextClicked()
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingDataSlideLearnMoreCaption
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
        onClicked: MZUrlOpener.openUrlLabel("privacyNotice")
    }

}
