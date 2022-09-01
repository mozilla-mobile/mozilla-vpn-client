/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import components 0.1
import telemetry 0.30

VPNViewBase {
    id: root
    objectName: "settingsTipsAndTricksPage"

    _menuTitle: VPNl18n.TipsAndTricksSettingsEntryLabel
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
                property string title: VPNl18n.TipsAndTricksQuickTipsTitle
                property string description: VPNl18n.TipsAndTricksQuickTipsDescription
                property var filter: (addon) => !addon.advanced && !addon.highlighted
            }

            // Advanced tips
            Item {
                property string title: VPNl18n.TipsAndTricksAdvancedTipsTitle
                property string description: VPNl18n.TipsAndTricksAdvancedTipsDescription
                property var filter: (addon) => addon.advanced && !addon.highlighted
            }
        }

        ObjectModel {
            id: tutorialsSections

            // Highlighted tutorials
            Item {
                property string title: VPNl18n.TipsAndTricksTutorialsTitle
                property string description: VPNl18n.TipsAndTricksTutorialsDescription
                property var filter: (addon) => addon.highlighted
                property var featureTourCardVisible: true
            }

            // More tutorials
            Item {
                property string title: VPNl18n.TipsAndTricksMoreTutorialsTitle
                property string description: VPNl18n.TipsAndTricksMoreTutorialsDescription
                property var filter: (addon) => !addon.highlighted
            }
        }

        stackContent: [
            // All
            VPNFlickable {
                objectName: 'allFlickable'

                flickContentHeight: layoutAll.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                ColumnLayout {
                    id: layoutAll

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    Repeater {
                        model: 2

                        delegate: Column {
                            Layout.fillWidth: true
                            Layout.topMargin: VPNTheme.theme.vSpacing
                            spacing: VPNTheme.theme.vSpacing

                            VPNTipsAndTricksSection {
                                anchors.right: parent.right
                                anchors.left: parent.left

                                property var section: tutorialsSections.get(index)

                                title: section.title
                                description: section.description

                                type: "tutorials"
                                customFilter: section.filter
                                featureTourCardVisible: section.featureTourCardVisible
                            }

                            VPNTipsAndTricksSection {
                                anchors.right: parent.right
                                anchors.left: parent.left

                                property var section: guidesSections.get(index)

                                title: section.title
                                description: section.description

                                type: "guides"
                                customFilter: section.filter
                                featureTourCardVisible: section.featureTourCardVisible
                            }
                        }
                    }

                    VPNFooterMargin {}
                }
            },

            // Tutorials
            VPNFlickable {
                flickContentHeight: layoutTutorial.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                VPNTutorialList {
                    id: layoutTutorial

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.bottomMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    VPNFooterMargin {}
                }
            },

            // Tips
            VPNFlickable {
                flickContentHeight: layoutGuide.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                ColumnLayout {
                    id: layoutGuide

                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    Repeater {
                        model: guidesSections.count

                        delegate: VPNTipsAndTricksSection {
                            Layout.fillWidth: true
                            Layout.topMargin: VPNTheme.theme.vSpacing

                            property var section: guidesSections.get(index)

                            title: section.title
                            description: section.description

                            type: "guides"
                            customFilter: section.filter
                            featureTourCardVisible: section.featureTourCardVisible
                        }
                    }

                    VPNFooterMargin {}
                }
            }
        ]

        Component.onCompleted: {
            Sample.tipsAndTricksViewOpened.record();
        }
    }
}
