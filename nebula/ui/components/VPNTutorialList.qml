/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

GridLayout {
    property int parentWidth
    property bool showVPNTour: true
    property var customTutorialFilter: () => true
    property var count: tutorialRepeater.count + (showVPNTour ? 1 : 0)

    Layout.alignment: Qt.AlignTop

    width: parentWidth
    columns: parentWidth < VPNTheme.theme.tabletMinimumWidth ? 1 : 2
    columnSpacing: VPNTheme.theme.vSpacingSmall
    rowSpacing: VPNTheme.theme.vSpacingSmall

    function tutorialFilter(addon) {
        return addon.type === "tutorial" && customTutorialFilter(addon);
    }

    VPNFilterProxyModel {
        id: tutorialModel
        source: VPNAddonManager
        filterCallback: ({ addon }) => tutorialFilter(addon)
    }

    Repeater {
        id: tutorialRepeater
        model: tutorialModel

        delegate: VPNTutorialCard {
            objectName: addon.id

            Layout.preferredHeight: VPNTheme.theme.tutorialCardHeight
            Layout.fillWidth: true

            imageSrc: addon.image
            title: addon.title
            description: addon.subtitle
            onClicked: {
                VPNTutorial.play(addon);
                VPNCloseEventHandler.removeAllStackViews();
            }
        }
    }

    VPNTutorialCard {
        objectName: "featureTourCard"

        Layout.preferredHeight: VPNTheme.theme.tutorialCardHeight
        Layout.fillWidth: true

        visible: showVPNTour

        imageSrc: "qrc:/ui/resources/sparkling-check.svg"
        imageBgColor: "#2B2A33"
        title: VPNl18n.TipsAndTricksFeatureTourCardTitle
        description: VPNl18n.TipsAndTricksFeatureTourCardDescription
        onClicked: featureTourPopup.startTour();
  }
}
