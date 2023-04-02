/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import Mozilla.Shared 1.0
import Mozilla.Shared.qmlcomponents 1.0
import Mozilla.VPN 1.0
import components 0.1

GridLayout {
    objectName: "tutorialList"

    property var customFilter: () => true
    property var count: tutorialRepeater.count

    columns: width < MZTheme.theme.tabletMinimumWidth ? 1 : 2
    columnSpacing: MZTheme.theme.vSpacingSmall
    rowSpacing: MZTheme.theme.vSpacingSmall
    visible: count > 0

    function tutorialFilter(addon) {
        return !!addon.as(MZAddon.TypeTutorial) && customFilter(addon);
    }

    MZFilterProxyModel {
        id: tutorialModel
        source: MZAddonManager
        filterCallback: ({ addon }) => tutorialFilter(addon)
    }

    Repeater {
        id: tutorialRepeater
        model: tutorialModel

        delegate: MZTutorialCard {
            property var addonTutorial: addon.as(MZAddon.TypeTutorial)

            objectName: addonTutorial.highlighted ? "highlightedTutorial" : addonTutorial.id

            Layout.preferredHeight: MZTheme.theme.tutorialCardHeight
            Layout.fillWidth: true

            imageSrc: addonTutorial.image
            title: addonTutorial.title
            description: addonTutorial.subtitle
            onClicked: {
                if (addonTutorial.settingsRollbackNeeded) {
                    MZTutorial.showWarning(addonTutorial)
                }
                else {
                    MZTutorial.play(addon);
                    MZNavigator.requestScreen(VPN.ScreenHome)
                }
            }
        }
    }
}
