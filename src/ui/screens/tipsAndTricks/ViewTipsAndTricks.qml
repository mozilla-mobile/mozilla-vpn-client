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
            id: tipsSections

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
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin
                    spacing: VPNTheme.theme.vSpacingSmall

                    VPNTutorialList {
                        Layout.fillWidth: true

                        objectName: "highlightedTutorials"
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
                        model: tipsSections.count

                        Column {
                            property var section: tipsSections.get(index)

                            visible: guidesList.count || tutorialsList.count

                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: VPNTheme.theme.vSpacingSmall
                            spacing: VPNTheme.theme.vSpacing
                            objectName: 'guideLayout'

                            VPNGuideList {
                                anchors.left: parent.left
                                anchors.right: parent.right

                                id: guidesList
                                title: section.title
                                description: section.description
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

                VPNTutorialList {
                    id: layoutTutorial

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.bottomMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin
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
                    anchors.topMargin: VPNTheme.theme.vSpacing
                    anchors.leftMargin: VPNTheme.theme.windowMargin
                    anchors.rightMargin: VPNTheme.theme.windowMargin

                    spacing: VPNTheme.theme.vSpacing

                    Repeater {
                        model: tipsSections.count

                        VPNGuideList {
                            property var section: tipsSections.get(index)

                            visible: !!count

                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignTop

                            title: section.title
                            description: section.description
                            customGuideFilter: section.filter
                        }
                    }
                }
            }
        ]

        Component.onCompleted: {
            Sample.tipsAndTricksViewOpened.record();
        }
    }
}
