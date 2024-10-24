/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import Mozilla.VPN 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    _menuTitle: "Telemetry Debugging"
    _viewContentData: ColumnLayout {
        id: telemetryDebuggingHolder
        spacing: MZTheme.theme.windowMargin
        Layout.fillWidth: true

        MZCheckBoxRow {
            id: checkBoxRowGleanLogPings

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText: "Glean Log Pings"
            subLabelText: "When this settings is turned on, pings will be logged to the console upon submission. This will not be synced with the mobile daemon Glean instances."
            isChecked: MZSettings.gleanLogPings
            showDivider: false
            onClicked: {
                VPN.gleanSetLogPings(!MZSettings.gleanLogPings)
            }
        }

        MZCheckBoxRow {
            id: checkBoxRowGleanDebugTag

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText: "Glean Debug View Tag"
            subLabelText: "Set Glean debug view tag for testing. Must restart app to take effect."
            isChecked: MZSettings.gleanDebugTagActive
            showDivider: false
            onClicked: {
                MZSettings.gleanDebugTagActive = !MZSettings.gleanDebugTagActive;
            }
        }

        MZTextField {
            id: gleanDebugTagString

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin * 2
            Layout.leftMargin: MZTheme.theme.windowMargin * 3

            Layout.alignment: Qt.AlignHCenter
            enabled: MZSettings.isGleanDebugTagActive
            _placeholderText: "VPNTest"
            Layout.preferredHeight: MZTheme.theme.rowHeight

            PropertyAnimation on opacity {
                duration: 200
            }

            onFocusChanged: {
                // Apply the new tag on focus out
                if (!focus) VPN.gleanSetDebugViewTag(gleanDebugTagString.text)
            }

            Component.onCompleted: {
                gleanDebugTagString.text = MZSettings.gleanDebugTag;
            }
        }

        MZCheckBoxRow {
            id: checkBoxRowShortSessionTimer

            Layout.fillWidth: true
            Layout.rightMargin: MZTheme.theme.windowMargin
            labelText: "3 minute timers for vpnsession and daemonsession pings"
            subLabelText: "Session pings typically send every 3 hours when a session is active. This shortens the period to 3 minutes. Will take effect on subsequent VPN activation."
            isChecked: MZSettings.shortTimerSessionPing
            showDivider: false
            onClicked: {
                MZSettings.shortTimerSessionPing = !MZSettings.shortTimerSessionPing
            }
        }

        Rectangle {
            id: divider
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            Layout.topMargin: MZTheme.theme.windowMargin / 2
            Layout.leftMargin: MZTheme.theme.windowMargin * 3
            Layout.rightMargin: MZTheme.theme.windowMargin
            color: MZTheme.colors.grey10
        }

        MZButton {
            id: submitMainPing
            text: "Submit the 'main' ping"
            onClicked: GleanPings.Main.submit()
        }

        MZButton {
            id: submitSessionPing
            text: "Submit the 'vpnsession' ping"
            onClicked: GleanPings.Vpnsession.submit()
        }
    }
}
