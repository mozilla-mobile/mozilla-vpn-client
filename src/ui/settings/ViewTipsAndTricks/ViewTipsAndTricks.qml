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

    // The list of add-on guides
    VPNFilterProxyModel {
        id: guideModel
        source: VPNAddonManager
        filterCallback: obj => obj.addon.type === "guide"
    }

    //Model containing all tutorials except the highlighted one (if there are any more)
    VPNFilterProxyModel {
        id: highlightedTutorialExcludedModel
        source: VPNAddonManager
        filterCallback: obj => obj.addon.type === "tutorial" && !obj.addon.highlighted;
    }

    // Highlighted tutorials
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
            TipsAndTricksSection {
                id: quicksTips

                flickableParent: vpnFlickable

                title: VPNl18n.TipsAndTricksQuickTipsTitle
                description: VPNl18n.TipsAndTricksQuickTipsDescription

                hasGuides: !!VPNAddonManager.pick(addon => addon.type === "guide")
                guidesModel: guideModel

                hasTutorials: !!VPNAddonManager.pick(addon => addon.type === "tutorial" && !addon.highlighted)
                tutorialsModel: highlightedTutorialExcludedModel
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
