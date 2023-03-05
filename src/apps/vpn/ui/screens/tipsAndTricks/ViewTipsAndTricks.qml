/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import components 0.1
import telemetry 0.30

MZViewBase {
    id: root
    objectName: "settingsTipsAndTricksPage"

    _menuTitle: MZI18n.SettingsTipsAndTricksSettings
    _interactive: false

    _viewContentData: MZTabNavigation {
        id: tabs
        Layout.topMargin: -MZTheme.theme.windowMargin
        Layout.preferredHeight: root.height - MZTheme.theme.menuHeight
        Layout.preferredWidth: root.width

        tabList: ListModel {
            id: tabButtonList
            ListElement {
                tabLabelStringId: "TipsAndTricksTabTitleAll"
                tabButtonId: "tabAll"
            }
            ListElement {
                tabLabelStringId:"TipsAndTricksTabTitleTutorials"
                tabButtonId: "tabTutorials"
            }
            ListElement {
                tabLabelStringId:"TipsAndTricksTabTitleTips"
                tabButtonId: "tabTips"
            }
        }

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

        ObjectModel {
            id: tutorialsSections

            // Highlighted tutorials
            Item {
                property string title: MZI18n.TipsAndTricksTutorialsTitle
                property string description: MZI18n.TipsAndTricksTutorialsDescription
                property var filter: (addon) => addon.highlighted
            }

            // More tutorials
            Item {
                property string title: MZI18n.TipsAndTricksMoreTutorialsTitle
                property string description: MZI18n.TipsAndTricksMoreTutorialsDescription2
                property var filter: (addon) => !addon.highlighted
            }
        }

        stackContent: [
            // All
            MZViewBase {
                objectName: 'allTab'
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutAll
                    objectName: 'layoutAll'

                    Layout.fillWidth: true
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin
                    Layout.topMargin: 8
                    spacing: MZTheme.theme.vSpacing

                    Repeater {
                        model: 2

                        delegate: Column {
                            Layout.fillWidth: true
                            spacing: MZTheme.theme.vSpacing

                            objectName: 'columnAll'

                            MZTipsAndTricksSection {
                                objectName: 'tutorialsSectionAll'

                                anchors.right: parent.right
                                anchors.left: parent.left

                                property var section: tutorialsSections.get(index)

                                title: section.title
                                description: section.description

                                type: "tutorials"
                                customFilter: section.filter
                            }

                            MZTipsAndTricksSection {
                                anchors.right: parent.right
                                anchors.left: parent.left

                                property var section: guidesSections.get(index)

                                title: section.title
                                description: section.description

                                type: "guides"
                                customFilter: section.filter
                            }
                        }
                    }
                }
            },

            // Tutorials
            MZViewBase {
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutTutorial

                    Layout.fillWidth: true
                    Layout.leftMargin: MZTheme.theme.windowMargin
                    Layout.rightMargin: MZTheme.theme.windowMargin
                    Layout.topMargin: 8
                    spacing: MZTheme.theme.vSpacing

                    Repeater {
                        model: tutorialsSections.count

                        delegate: MZTipsAndTricksSection {
                            Layout.fillWidth: true

                            property var section: tutorialsSections.get(index)

                            title: section.title
                            description: section.description

                            type: "tutorials"
                            customFilter: section.filter
                        }
                    }
                }
            },

            // Tips
            MZViewBase {
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutGuide

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
            }
        ]

        Component.onCompleted: {
            Sample.tipsAndTricksViewOpened.record();
            Glean.sample.tipsAndTricksViewOpened.record();
        }
    }
}
