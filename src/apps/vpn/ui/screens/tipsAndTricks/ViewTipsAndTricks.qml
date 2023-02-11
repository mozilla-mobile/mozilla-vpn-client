/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import components 0.1
import telemetry 0.30

VPNViewBase {
    id: root
    objectName: "settingsTipsAndTricksPage"

    _menuTitle: VPNI18n.SettingsTipsAndTricksSettings
    _interactive: false

    _viewContentData: VPNTabNavigation {
        id: tabs
        Layout.topMargin: -VPNTheme.theme.windowMargin
        Layout.preferredHeight: root.height - VPNTheme.theme.menuHeight
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
                property string title: VPNI18n.TipsAndTricksQuickTipsTitle
                property string description: VPNI18n.TipsAndTricksQuickTipsDescription
                property var filter: (addon) => !addon.advanced && !addon.highlighted
            }

            // Advanced tips
            Item {
                property string title: VPNI18n.TipsAndTricksAdvancedTipsTitle
                property string description: VPNI18n.TipsAndTricksAdvancedTipsDescription
                property var filter: (addon) => addon.advanced && !addon.highlighted
            }
        }

        ObjectModel {
            id: tutorialsSections

            // Highlighted tutorials
            Item {
                property string title: VPNI18n.TipsAndTricksTutorialsTitle
                property string description: VPNI18n.TipsAndTricksTutorialsDescription
                property var filter: (addon) => addon.highlighted
            }

            // More tutorials
            Item {
                property string title: VPNI18n.TipsAndTricksMoreTutorialsTitle
                property string description: VPNI18n.TipsAndTricksMoreTutorialsDescription2
                property var filter: (addon) => !addon.highlighted
            }
        }

        stackContent: [
            // All
            VPNViewBase {
                objectName: 'allTab'
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutAll
                    objectName: 'layoutAll'

                    Layout.fillWidth: true
                    Layout.leftMargin: VPNTheme.theme.windowMargin
                    Layout.rightMargin: VPNTheme.theme.windowMargin
                    Layout.topMargin: 8
                    spacing: VPNTheme.theme.vSpacing

                    Repeater {
                        model: 2

                        delegate: Column {
                            Layout.fillWidth: true
                            spacing: VPNTheme.theme.vSpacing

                            objectName: 'columnAll'

                            VPNTipsAndTricksSection {
                                objectName: 'tutorialsSectionAll'

                                anchors.right: parent.right
                                anchors.left: parent.left

                                property var section: tutorialsSections.get(index)

                                title: section.title
                                description: section.description

                                type: "tutorials"
                                customFilter: section.filter
                            }

                            VPNTipsAndTricksSection {
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
            VPNViewBase {
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutTutorial

                    Layout.fillWidth: true
                    Layout.leftMargin: VPNTheme.theme.windowMargin
                    Layout.rightMargin: VPNTheme.theme.windowMargin
                    Layout.topMargin: 8
                    spacing: VPNTheme.theme.vSpacing

                    Repeater {
                        model: tutorialsSections.count

                        delegate: VPNTipsAndTricksSection {
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
            VPNViewBase {
                anchors.top: undefined

                _viewContentData: ColumnLayout {
                    id: layoutGuide

                    Layout.fillWidth: true
                    Layout.leftMargin: VPNTheme.theme.windowMargin
                    Layout.rightMargin: VPNTheme.theme.windowMargin
                    Layout.topMargin: 8
                    spacing: VPNTheme.theme.vSpacing

                    Repeater {
                        model: guidesSections.count

                        delegate: VPNTipsAndTricksSection {
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
