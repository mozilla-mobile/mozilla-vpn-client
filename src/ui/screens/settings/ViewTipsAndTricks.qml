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

        stackContent: [
            // All
            VPNFlickable {
                flickContentHeight: layoutAll.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                ObjectModel {
                    id: tipsAndTricksSections

                    Item { 
                        property string title: VPNl18n.TipsAndTricksQuickTipsTitle
                        property string description: VPNl18n.TipsAndTricksQuickTipsDescription
                        property var filter: (addon) => !addon.advanced && !addon.highlighted
                    }

                    Item { 
                        property string title: VPNl18n.TipsAndTricksAdvancedTipsTitle
                        property string description: VPNl18n.TipsAndTricksAdvancedTipsDescription
                        property var filter: (addon) => addon.advanced && !addon.highlighted
                    }
                }

                ColumnLayout {
                    id: layoutAll

                    anchors.fill: parent
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.bottomMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin
                    spacing: VPNTheme.theme.vSpacingSmall

                    VPNTutorialList {
                        customTutorialFilter: (addon) => addon.highlighted
                    }

                    VPNTutorialCard {
                        objectName: "featureTourCard"

                        Layout.preferredHeight: VPNTheme.theme.tutorialCardHeight
                        Layout.fillWidth: true

                        imageSrc: "qrc:/ui/resources/sparkling-check.svg"
                        imageBgColor: "#2B2A33"
                        title: VPNl18n.TipsAndTricksFeatureTourCardTitle
                        description: VPNl18n.TipsAndTricksFeatureTourCardDescription
                        onClicked: featureTourPopup.startTour();
                    }

                    Repeater {
                        model: tipsAndTricksSections.count

                        Column {
                            property var section: tipsAndTricksSections.get(index)

                            visible: guidesList.count || tutorialsList.count

                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 16
                            spacing: VPNTheme.theme.vSpacingSmall

                            VPNBoldLabel {
                                anchors.left: parent.left
                                anchors.right: parent.right
                                
                                text: section.title
                                wrapMode: Text.WordWrap

                                Accessible.role: Accessible.StaticText
                                Accessible.name: text
                            }

                            VPNTextBlock {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                text: section.description
                                wrapMode: Text.WordWrap

                                Accessible.role: Accessible.StaticText
                                Accessible.name: text
                            }

                            VPNGuideList {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                id: guidesList
                                customGuideFilter: section.filter
                            }

                            VPNTutorialList {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                id: tutorialsList
                                customTutorialFilter: section.filter
                            }
                        }
                    }
                }
            },

            // Tutorials
            VPNFlickable {
                flickContentHeight: layoutTutorial.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                ColumnLayout {
                    id: layoutTutorial

                    anchors.fill: parent
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.bottomMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    VPNTutorialList { }
                }
            },

            // Tips
            VPNFlickable {
                flickContentHeight: layoutGuide.implicitHeight + (VPNTheme.theme.vSpacing * 2)
                interactive: flickContentHeight > height

                ColumnLayout {
                    id: layoutGuide

                    anchors.fill: parent
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.bottomMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    VPNGuideList { }
                }
            }
        ]

        Component.onCompleted: {
            Sample.tipsAndTricksViewOpened.record();
        }
    }
}
