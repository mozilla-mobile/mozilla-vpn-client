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
    objectName: "settingsAppearanceView"

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

        Repeater {
            model: ListModel {
                id: appearanceMenu

                ListElement {
                    textName: "SettingsAppearanceAutomatic"
                    radioButtonName: "automaticAppearanceRadioButton"
                    radioButtonId: "automaticRadioButton"
                }

                ListElement {
                    textName: "SettingsAppearanceLight"
                    radioButtonName: "lightAppearanceRadioButton"
                    radioButtonId: "lightRadioButton"
                }

                ListElement {
                    textName: "SettingsAppearanceDark"
                    radioButtonName: "darkAppearanceRadioButton"
                    radioButtonId: "darkRadioButton"
                }
            }

            delegate: RowLayout {
                spacing: MZTheme.theme.windowMargin
                Layout.rightMargin: MZTheme.theme.windowMargin / 2
                visible: isVisible(radioButtonName)

                MZRadioButton {
                    objectName: radioButtonName

                    id: radioButtonId
                    Layout.preferredWidth: MZTheme.theme.vSpacing
                    Layout.alignment: Qt.AlignTop
                    checked: isChecked(radioButtonName)
                    ButtonGroup.group: radioButtonGroup
                    accessibleName: MZI18n[textName]
                    onClicked: wasClicked(radioButtonName)
                }

                ColumnLayout {
                    spacing: 4
                    Layout.alignment: Qt.AlignTop

                    MZInterLabel {
                        Layout.fillWidth: true

                        text: MZI18n[textName]
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignLeft

                        MZMouseArea {
                            anchors.fill: parent

                            enabled: radioButtonId.enabled
                            width: Math.min(parent.implicitWidth, parent.width)
                            propagateClickToParent: false
                            onClicked: wasClicked(radioButtonName)
                        }
                    }

                    MZTextBlock {
                        text: MZI18n.SettingsAppearanceAutomaticDescription
                        Layout.fillWidth: true
                        visible: radioButtonName === "automaticAppearanceRadioButton"
                    }
                }
            }
        }
    }

    function isChecked(buttonId) {
      switch (buttonId) {
        case "automaticAppearanceRadioButton":
            return MZSettings.usingSystemTheme
        case "darkAppearanceRadioButton":
            return MZTheme.currentTheme == "dark-mode" && !MZSettings.usingSystemTheme
        case "lightAppearanceRadioButton":
            return MZTheme.currentTheme == "main" && !MZSettings.usingSystemTheme
        default:
            console.error("Unable to find radio button type: " + buttonId)
            return false
        }
    }

    function wasClicked(buttonId) {
      switch (buttonId) {
        case "automaticAppearanceRadioButton":
            MZTheme.setUsingSystemTheme(true)
            return
        case "darkAppearanceRadioButton":
            setTheme("dark-mode")
            return
        case "lightAppearanceRadioButton":
            setTheme("main")
            return
        default:
            console.error("Unable to find radio button type: " + buttonId)
        }
    }

    function isVisible(buttonId) {
      if (buttonId === "automaticAppearanceRadioButton") {
        return MZFeatureList.get("themeSelectionIncludesAutomatic").isSupported
      }

      return true
    }

    function setTheme(newTheme) {
        MZTheme.setUsingSystemTheme(false)
        MZTheme.currentTheme = newTheme
    }
}
