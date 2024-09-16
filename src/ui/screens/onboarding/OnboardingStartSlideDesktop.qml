/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import "qrc:/nebula/utils/MZUiUtils.js" as MZUiUtils

ColumnLayout {
    id: root
    objectName: "onboardingStartSlide"

    property string telemetryScreenId: "connect_on_startup"
    // `startAtBootCheckbox` was added to change the default `startAtBoot` setting to true. We  couldn't simply change the `startAtBoot` default in `settingslist.h`
    // without affecting older VPN clients. See https://github.com/mozilla-mobile/mozilla-vpn-client/pull/9878
    property bool startAtBootCheckbox: true

    signal nextClicked()
    signal backClicked()

    spacing: 0

    MZHeadline {
        Layout.topMargin: MZTheme.theme.windowMargin * 1.5
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5

        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideHeader
        horizontalAlignment: Text.AlignLeft
    }

    MZInterLabel {
        Layout.topMargin: 8
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        text: MZI18n.OnboardingStartSlideBody2
        horizontalAlignment: Text.AlignLeft
        color: MZTheme.theme.fontColor
    }

    RowLayout {
        Layout.topMargin: MZTheme.theme.vSpacingSmall
        Layout.leftMargin: MZTheme.theme.windowMargin * 1.5
        Layout.rightMargin: MZTheme.theme.windowMargin * 1.5
        Layout.fillWidth: true

        MZBoldInterLabel {
            id: connectOnStartupLabel
            Layout.fillWidth: true

            text: MZI18n.SettingsStartAtBootTitle
            font.pixelSize: MZTheme.theme.fontSize
            lineHeight: MZTheme.theme.labelLineHeight
        }

        MZSettingsToggle {
            objectName: "startAtBootToggle"

            checked: startAtBootCheckbox
            onClicked: {
                startAtBootCheckbox = !startAtBootCheckbox

                if (startAtBootCheckbox) {
                    Glean.interaction.connectOnStartupEnabled.record({
                        screen: root.telemetryScreenId,
                    });
                }
                else {
                    Glean.interaction.connectOnStartupDisabled.record({
                        screen: root.telemetryScreenId,
                    });
                }
            }

            accessibleName: connectOnStartupLabel.text
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    Image {
        Layout.alignment: Qt.AlignHCenter

        sourceSize: MZUiUtils.isLargePhone() ? Qt.size(209,144) : Qt.size(151,104)
        source: "qrc:/ui/resources/launch.svg"
    }

    Item {
        Layout.fillHeight: true
        Layout.minimumHeight: MZTheme.theme.onboardingMinimumVerticalSpacing
    }

    MZButton {
        objectName: "startNextButton"

        Layout.leftMargin: MZTheme.theme.windowMargin * 2
        Layout.rightMargin: MZTheme.theme.windowMargin * 2
        Layout.fillWidth: true

        width: undefined
        text: MZI18n.GlobalGetStarted

        onClicked: {
            MZSettings.startAtBoot = startAtBootCheckbox

            Glean.interaction.getStartedSelected.record({
                screen: root.telemetryScreenId,
            });

            VPN.onboardingCompleted();
        }
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

        onClicked: {
            Glean.interaction.goBackSelected.record({
                screen: root.telemetryScreenId,
            });

            root.backClicked()
        }
    }
}
