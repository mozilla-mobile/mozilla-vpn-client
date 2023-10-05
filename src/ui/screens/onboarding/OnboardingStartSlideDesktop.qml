/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1

ColumnLayout {
    id: root
    objectName: "onboardingStartSlide"

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2

        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideBody
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 24
    }

    Image {
        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.alignment: Qt.AlignHCenter

        source: "qrc:/ui/resources/launch.svg"
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 24
    }

    MZCheckBoxRow {
        objectName: "startAtBootCheckBox"

        Layout.topMargin: MZTheme.theme.vSpacing
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        subLabelText: MZI18n.SettingsStartAtBootTitle
        leftMargin: 0
        isChecked: MZSettings.startAtBoot
        showDivider: false
        onClicked: MZSettings.startAtBoot = !MZSettings.startAtBoot
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: 48
    }

    MZButton {
        objectName: "startNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.GlobalGetStarted

        onClicked: VPN.onboardingCompleted();
    }

    MZLinkButton {
        objectName: "startBackButton"

        Layout.topMargin: 16
        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.bottomMargin: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        implicitHeight: MZTheme.theme.rowHeight

        labelText: MZI18n.GlobalGoBack

        onClicked: root.backClicked()
    }
}
