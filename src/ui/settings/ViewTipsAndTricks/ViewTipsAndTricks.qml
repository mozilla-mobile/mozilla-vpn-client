/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1
import components.forms 0.1
import telemetry 0.30

VPNFlickable {
    id: vpnFlickable

    property string _menuTitle: VPNl18n.TipsAndTricksSettingsEntryLabel

    objectName: "settingsTipsAndTricksPage"

    flickContentHeight: layout.implicitHeight + layout.anchors.topMargin
    interactive: flickContentHeight > height

    function isQuickAddon(addon, type) {
        return (addon.type === type && !addon.advanced && !addon.highlighted)
    }

    function isAdvancedAddon(addon, type) {
        return (addon.type === type && addon.advanced && !addon.highlighted)
    }

    Column {
        id: layout
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: VPNTheme.theme.vSpacing
        anchors.leftMargin: VPNTheme.theme.windowMargin
        anchors.rightMargin: VPNTheme.theme.windowMargin

        spacing: 0

        //superfluous columnlayout that fixes a binding loop
        ColumnLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 0

            // Highlighted tutorials
            Flow {
                Layout.fillWidth: true
                spacing: VPNTheme.theme.vSpacingSmall

                Loader {
                    id: highlightedTutorialLoader
                    property variant highlightedTutorial: VPNAddonManager.pick(addon => addon.type === "tutorial" && addon.highlighted)

                    height: VPNTheme.theme.tutorialCardHeight
                    width: vpnFlickable.width < VPNTheme.theme.tabletMinimumWidth ? parent.width : (parent.width - parent.spacing) / 2

                    active: highlightedTutorial
                    visible: active

                    sourceComponent: VPNTutorialCard {
                        objectName: "highlightedTutorial"
                        width: parent.width
                        height: parent.height

                        imageSrc: highlightedTutorial.image
                        title: qsTrId(highlightedTutorial.titleId)
                        description: qsTrId(highlightedTutorial.subtitleId)

                        onClicked: {
                            VPNTutorial.play(highlightedTutorial);
                            VPNCloseEventHandler.removeAllStackViews();
                        }
                    }
                }

                VPNTutorialCard {
                    objectName: "featureTourCard"

                    width: vpnFlickable.width < VPNTheme.theme.tabletMinimumWidth ? parent.width : (parent.width - parent.spacing) / 2
                    height: VPNTheme.theme.tutorialCardHeight

                    imageSrc: "qrc:/ui/resources/sparkling-check.svg"
                    imageBgColor: "#2B2A33"
                    title: VPNl18n.TipsAndTricksFeatureTourCardTitle
                    description: VPNl18n.TipsAndTricksFeatureTourCardDescription
                    onClicked: featureTourPopup.startTour();
                }
            }

            // Quick Tips
            VPNFilterProxyModel {
                id: quickGuidesModel
                source: VPNAddonManager
                filterCallback: ({ addon }) => isQuickAddon(addon, "guide")
            }

            VPNFilterProxyModel {
                id: quickTutorialsModel
                source: VPNAddonManager
                filterCallback: ({ addon }) => isQuickAddon(addon, "tutorial")
            }

            TipsAndTricksSection {
                id: quicksTips

                Layout.topMargin: 32

                parentWidth: vpnFlickable.width

                title: VPNl18n.TipsAndTricksQuickTipsTitle
                description: VPNl18n.TipsAndTricksQuickTipsDescription

                hasGuides: !!VPNAddonManager.pick(addon => isQuickAddon(addon, "guide"))
                guidesModel: quickGuidesModel

                hasTutorials: !!VPNAddonManager.pick(addon => isQuickAddon(addon, "tutorial"))
                tutorialsModel: quickTutorialsModel
            }

            // Advanced Tips
            VPNFilterProxyModel {
                id: advancedGuidesModel
                source: VPNAddonManager
                filterCallback: ({ addon }) => isAdvancedAddon(addon, "guide")
            }

            VPNFilterProxyModel {
                id: advancedTutorialsModel
                source: VPNAddonManager
                filterCallback: ({ addon }) => isAdvancedAddon(addon, "tutorial")
            }

            TipsAndTricksSection {
                id: advancedsTips

                Layout.topMargin: 32

                parentWidth: vpnFlickable.width

                title: VPNl18n.TipsAndTricksAdvancedTipsTitle
                description: VPNl18n.TipsAndTricksAdvancedTipsDescription

                hasGuides: !!VPNAddonManager.pick(addon => isAdvancedAddon(addon, "guide"))
                guidesModel: advancedGuidesModel

                hasTutorials: !!VPNAddonManager.pick(addon => isAdvancedAddon(addon, "tutorial"))
                tutorialsModel: advancedTutorialsModel
            }

            //padding for the bottom of the flickable
            Item {
                Layout.preferredHeight: 66
            }
        }
    }

    Component.onCompleted: {
        Sample.tipsAndTricksViewOpened.record();
    }
}
