/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.5
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import Mozilla.Shared 1.0
import components 0.1
import components.forms 0.1

MZViewBase {
    id: root
    objectName: "AppearanceView"

    _menuTitle: MZI18n.SettingsAppearance

    readonly property string telemetryScreenId : "appearance_settings"
    
    _viewContentData: ColumnLayout {
        spacing: MZTheme.theme.windowMargin
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin
        Layout.topMargin: MZTheme.theme.windowMargin
        Layout.preferredWidth: parent.width

        ButtonGroup {
            id: radioButtonGroup
        }

        RowLayout {
            spacing: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin / 2
            visible: MZFeatureList.get("themeSelectionIncludesAutomatic").isSupported

            MZRadioButton {
                objectName: "automatic"

                id: automaticRadioButton
                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.preferredHeight: MZTheme.theme.rowHeight
                Layout.alignment: Qt.AlignTop
                checked: MZTheme.usingSystemTheme
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsAppearanceAutomatic
                onClicked: MZTheme.usingSystemTheme = true
            }

            ColumnLayout {
                spacing: 4

                MZInterLabel {
                    Layout.fillWidth: true

                    text: MZI18n.SettingsAppearanceAutomatic
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    MZMouseArea {
                        anchors.fill: parent

                        enabled: automaticRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: MZTheme.usingSystemTheme = true
                    }
                }

                MZTextBlock {
                    text: MZI18n.SettingsAppearanceAutomaticDescription
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            spacing: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            MZRadioButton {
                objectName: "light"

                id: lightRadioButton
                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.alignment: Qt.AlignTop
                checked: MZTheme.currentTheme == "main" && !MZTheme.usingSystemTheme
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsAppearanceLight
                onClicked: setTheme("main")
            }   

            ColumnLayout {
                spacing: 4
                Layout.alignment: Qt.AlignTop

                MZInterLabel {
                    Layout.fillWidth: true

                    text: MZI18n.SettingsAppearanceLight
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    MZMouseArea {
                        anchors.fill: parent

                        enabled: lightRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: setTheme("main")
                    }
                }
            }
        }

        RowLayout {
            spacing: MZTheme.theme.windowMargin
            Layout.rightMargin: MZTheme.theme.windowMargin / 2

            MZRadioButton {
                objectName: "dark"

                id: darkRadioButton
                Layout.preferredWidth: MZTheme.theme.vSpacing
                Layout.alignment: Qt.AlignTop
                checked: MZTheme.currentTheme == "not-designer-approved" && !MZTheme.usingSystemTheme
                ButtonGroup.group: radioButtonGroup
                accessibleName: MZI18n.SettingsAppearanceDark
                onClicked: setTheme("not-designer-approved")
            }

            ColumnLayout {
                spacing: 4
                Layout.alignment: Qt.AlignTop

                MZInterLabel {
                    Layout.fillWidth: true

                    text: MZI18n.SettingsAppearanceDark
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignLeft

                    MZMouseArea {
                        anchors.fill: parent

                        enabled: darkRadioButton.enabled
                        width: Math.min(parent.implicitWidth, parent.width)
                        propagateClickToParent: false
                        onClicked: setTheme("not-designer-approved")
                    }
                }
            }
        }
    }

    function setTheme(newTheme) {
        MZTheme.usingSystemTheme = false
        MZTheme.currentTheme = newTheme
    }
}
