/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.VPN 1.0
import Mozilla.VPN.qmlcomponents 1.0
import components 0.1

GridLayout {
    objectName: "tutorialList"

    property var customFilter: () => true
    property var count: tutorialRepeater.count

    columns: width < VPNTheme.theme.tabletMinimumWidth ? 1 : 2
    columnSpacing: VPNTheme.theme.vSpacingSmall
    rowSpacing: VPNTheme.theme.vSpacingSmall
    visible: count > 0

    function tutorialFilter(addon) {
        return addon.type === "tutorial" && customFilter(addon);
    }

    MZFilterProxyModel {
        id: tutorialModel
        source: VPNAddonManager
        filterCallback: ({ addon }) => tutorialFilter(addon)
    }

    Repeater {
        id: tutorialRepeater
        model: tutorialModel

        delegate: VPNTutorialCard {
            objectName: addon.highlighted ? "highlightedTutorial" : addon.id

            Layout.preferredHeight: VPNTheme.theme.tutorialCardHeight
            Layout.fillWidth: true

            imageSrc: addon.image
            title: addon.title
            description: addon.subtitle
            onClicked: {
                if (addon.settingsRollbackNeeded) {
                    VPNTutorial.showWarning(addon)
                }
                else {
                    VPNTutorial.play(addon);
                    VPNNavigator.requestScreen(VPNNavigator.ScreenHome)
                }
            }
        }
    }
}
