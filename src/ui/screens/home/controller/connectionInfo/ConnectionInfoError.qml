/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import compat 0.1

ColumnLayout {
    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }
    width: parent.width - MZTheme.theme.windowMargin * 2

    Image {
        Layout.fillWidth: true

        source: "qrc:/ui/resources/connection-error-unstable.svg"
        fillMode: Image.PreserveAspectFit
    }

    MZMetropolisLabel {
        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.vSpacingSmall
        Layout.rightMargin: MZTheme.theme.windowMargin

        color: MZTheme.colors.white
        font.pixelSize: MZTheme.theme.fontSizeLarge
        text: MZI18n.GenericErrorUnexpected
        width: parent.width - MZTheme.theme.windowMargin
    }

    MZTextBlock {
        Layout.fillWidth: true
        Layout.bottomMargin: MZTheme.theme.vSpacingSmall
        Layout.leftMargin: MZTheme.theme.vSpacingSmall
        Layout.rightMargin: MZTheme.theme.windowMargin
        Layout.topMargin: MZTheme.theme.windowMargin

        color: MZTheme.colors.grey20
        horizontalAlignment: Text.AlignHCenter
        text: MZI18n.ConnectionInfoErrorMessage
        wrapMode: Text.WordWrap
        width: parent.width < MZTheme.theme.maxTextWidth
            ? parent.width
            : MZTheme.theme.maxTextWidth
    }

    MZButton {
        objectName: "connectionInfoErrorRetryButton"
        Layout.fillWidth: true

        // Try again
        text: MZI18n.GenericPurchaseErrorGenericPurchaseErrorButton
        onClicked: {
            Glean.interaction.speedTestRefresh.record({
                screen: "speed_test_error",
                action: "select",
                element_id: "try_again",
            });

            VPNConnectionBenchmark.start();
        }
    }
}
