/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1

MZViewBase {
    id: root
    objectName: "settingsTipsAndTricksPage"

    _menuTitle: MZI18n.SettingsTipsAndTricksSettings
    _interactive: true

    _viewContentData: ColumnLayout {
        id: layoutGuide

        ObjectModel {
            id: guidesSections

            // Quick tips
            Item {
                property string title: MZI18n.TipsAndTricksQuickTipsTitle
                property string description: MZI18n.TipsAndTricksQuickTipsDescription
                property var filter: (addon) => !addon.advanced && !addon.highlighted
            }

            // Advanced tips
            Item {
                property string title: MZI18n.TipsAndTricksAdvancedTipsTitle
                property string description: MZI18n.TipsAndTricksAdvancedTipsDescription
                property var filter: (addon) => addon.advanced && !addon.highlighted
            }
        }

        Layout.fillWidth: true
        Layout.leftMargin: MZTheme.theme.windowMargin
        Layout.rightMargin: MZTheme.theme.windowMargin
        Layout.topMargin: 8
        spacing: MZTheme.theme.vSpacing

        Repeater {
            model: guidesSections.count

            delegate: MZTipsAndTricksSection {
                Layout.fillWidth: true

                property var section: guidesSections.get(index)

                title: section.title
                description: section.description

                type: "guides"
                customFilter: section.filter
            }
        }
    }


    Component.onCompleted: {
        Glean.sample.tipsAndTricksViewOpened.record();
    }
}

